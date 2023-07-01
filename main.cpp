#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>
#include <sstream>
#include <set>

#include "txt_files.h"
#include "utf8.h"

/*
    Modules:

DataFrame
NormalDF - date, time, author, content
TimeDF - year, month, day, hour, minute, second(?)
KindDF - text, media, creation, removal, addition, title, ...
AlphabetsDF - length, english, emojis, hebrew, whitespace, punctuation, ...
CoindidenceDF - ioc

BlockDF - start_index, message_count, character_count

EntireDataFrame
LetterDF - character, amount
WordsDF - word, amount

DFFileParser
ChatFileParser
Alphabets
Colors
Filter
*/

/*
    Steps:

ChatFile reading
ChatFile parsing - messages list
df
ndf
DF file saving
tdf
cdf
filters
data export (to python for rendering)
*/

/*
    Conceptual:

I wanna parse each message (collection of lines) and immediately pass it to DFs' analyses.
So the whole thing is just one big iteration.
*/

// See std::wstring for multilanguage support

void handleFile(std::string);
void handleMessage(std::string, std::streampos, int);
bool isNewMessage(std::string, int);
bool isSeparator(char, int format);
void removeNonPositiveChars(std::string&);
bool startswith(const std::string&, const std::string&);
bool endswith(const std::string&, const std::string&);
bool contains(const std::string&, const std::string&);

enum MessageFormat {
    // DMY and MDY are exclusive
    DMY = 1,
    MDY = 2,
    // Slashes and dots are exclusive
    Slashes = 4,
    Dots = 8,
    // Parentheses and Dash are exclusive
    Parentheses = 16, // has to include seconds in the time signature ("hh:mm:ss]")
    Dash = 32 // just hours and minutes ("hh:mm - ")
};

enum MessageType : char {
    none, // An empty message (mostly software fault).
    text, // Regular text messages -- most messages sent in usual chats
    media, // <Media omitted>, or image / video / sticker
    deleted, // this message was deleted
    whatsapp_info, // whatsapp information
    created, // created a chat
    join, // a participant joined / was added
    leave, // a participant left / was kicked
    admin, // you are now admin
    block, // you blocked this contact
    unblock, // you unblocked this contact
    title, // title change
    description, // description change
    settings, // settings change
    icon // icon change
};
MessageType determineMessageType(std::string, std::string);

struct Message {
    int start;
    unsigned short length; // verified max length of a WhatsApp message

    // int author; // index in an authors array
    std::string author; // temporary

    short year; // -32 768 to 32 767
    char month;
    char day;
    char hour;
    char minute;

    float ioc;

    MessageType messageType;
};
void printMessage(Message*);

std::set<std::string> authors;
std::vector<Message*> messages;

int main() {
    std::vector<std::string> paths = getPaths("raw/");
    for (std::string path : paths)
    {
        if (!endswithTXT(path)) continue;
        std::cout << "Reading: " << path << std::endl; // improve this printing to call out "This is the chat with XYZ"
        
        handleFile(path);
    }

    return 0;
}

void handleFile(std::string path) {
    int messageFormat = DMY | Slashes | Parentheses; // later on I'll add automatic format detection. Right now it's hardcoded.

    std::string line;
    std::string message = "";
    std::ifstream f(path, std::ios::binary);
    assert(f.good());
    
    f.seekg(0);
    std::streampos start = currentPosition(f);
    std::streampos pos = currentPosition(f);
    while (std::getline(f, line, '\n')) {
        // Outsource to a function that determines whether a specific line is a `beginning` of a message or not.
        if (!line.empty() && line.back() == '\r')
            line.pop_back(); // remove `\r` characters
        if (line.empty() || line.back() != '\n') // The condition is a tad excessive (no `line` will have a `\n` already), but it's not problematic.
            line += "\n"; // add newline character.
        if (isNewMessage(line, messageFormat)) {
            handleMessage(message, start, messageFormat);
            // std::cout << message << std::endl;
            message = ""; // reset the `message` accumulator, to start a new message.
            start = pos; // reset the `start` of that new message to the [byte-after-last of the last-line of the previous-message].
        }
        message += line; // add a line to the message
        pos = currentPosition(f); // update the cursor position (placed at the end of `line`)
        // if ((int)pos > 1000) break; // artificial limitation, remove when you think.
        if (message.length() > 10000) // messages shall not be longer than 10000 characters, that would mean the file is not a chat
            break;
    }
    if (isNewMessage(message, messageFormat))
        handleMessage(message, start, messageFormat);
        // std::cout << message << std::endl;
    // TODO: get rid of `message`?
    f.clear();

    std::cout << "Participants:" << std::endl;
    for (const std::string& author : authors) { // also tagged authors and added participants?
        std::cout << author << std::endl;
    }
    authors.clear();

    for (const Message* msg : messages) {
        delete msg;
    }
    
    char buffer[69];
    extract(f, buffer, 528202); // extract 68 bytes from position 50 in the file `f`.
    // std::cout << buffer << std::endl;

    f.close();

    // don't forget to `delete` all the created Messages (array iteration O(1n)?).
}

void handleMessage(std::string content, std::streampos startpos, int format) {
    if (!content.empty() && content.back() == '\n')
        content.pop_back(); // remove trailing newline
    if (content.length() < 20) return; // invalid message, probably `@0 len=0` at the start of the file.
    
    Message* message = new Message();
    message->length = (unsigned short)content.length();
    message->start = (int)startpos;
    int startsWithParentheses = (int)(format & Parentheses && content[0] == '[');
    if (content[13 + startsWithParentheses] == ':') {
        // insert `0` between 11 and 12, then continue normally. (singular left zero-pad)
        content.insert(11, 1, '0');
    }
    int i = 0;
    std::stringstream datetime;
    for (i = 0; i <= 16; ++i) {
        datetime << content[i + startsWithParentheses];
    }

    int year, month, day, hour, minute;
    char _;
    datetime >> day >> _ >> month >> _ >> year >> _ >> hour >> _ >> minute;
    if (format & MDY) {
        int temp = month;
        month = day;
        day = temp;
    }

    message->year = year;
    message->month = month;
    message->day = day;
    message->hour = hour;
    message->minute = minute;

    std::stringstream author;
    bool found = false;
    for (i = (format & Parentheses) ? 23 : 20; i <= message->length; ++i) {
        if (content[i] == ':') {
            found = true;
            break;
        }
        author << content[i];
    }
    message->author = found ? author.str() : "WhatsApp";

    // actual message content
    std::string text;
    try {
        text = content.substr(i + 2);
    } catch (const std::out_of_range& e) {
        text = "";
    }

    if (found) {
        // ioc = sum(count * (count-1)) where count is every character's appearance count.
        // Divided by length * (length-1)
        // equivalent to `ioc = [sum(#²)-l]/[l²-l]`, where `#` is count and `l` is length.
        // 1. Get a list of counts. Which character each count is associated with -- doesn't matter. // how would I do this efficiently?
        // 2. Calculate the sum of squares.
        // 3. Plug into `message->ioc = [sum_of_squares-l]/[l²-l]`.
        message->ioc = 1.0f; // temporary
    } else {
        message->ioc = 0.0f; // WhatsApp's messages are uninteresting and shouldn't affect statistics.
    }

    authors.insert(message->author);
    message->messageType = determineMessageType(text, message->author);

    if (message->messageType != MessageType::text) {
        // printMessage(message);
        std::cout << content << std::endl;
    }

    messages.push_back(message);

    // save the message to a global array. // or,,, write to a `.csv` file? Depends on what I wanna do with the results
}

MessageType determineMessageType(std::string text, std::string author) {
    MessageType result = MessageType::none;
    if (text.length() == 0) return result;

    result = MessageType::text;

    removeNonPositiveChars(text);
    removeNonPositiveChars(author);

    // Run a bunch of equality checks to determine which type best describes the message
    if (text == "Messages and calls are end-to-end encrypted. No one outside of this chat, not even WhatsApp, can read or listen to them. Tap to learn more." ||
        text == "Messages and calls are end-to-end encrypted. No one outside of this chat, not even WhatsApp, can read or listen to them." ||
        text == "This chat is with a business account. Tap to learn more." ||
        text == "This chat is with a business account." ||
        author == "WhatsApp")
        result = MessageType::whatsapp_info;
    if (text == author + " created this group" ||
        text == author + " created this group." ||
        startswith(text, author + " created group"))
        result = MessageType::created;
    if (text == author + " joined using this group\'s invite link" ||
        endswith(text, " added you") ||
        endswith(text, " added " + author) ||
        contains(text, " added "))
        result = MessageType::join;
    if (startswith(text, author + " changed the subject to"))
        result = MessageType::title;
    if (text == author + " changed this group\'s icon")
        result = MessageType::icon;
    if (text == author + " changed the group description")
        result = MessageType::description;
    if (text == author + " left")
        result = MessageType::leave;
    if (text == author + " changed the settings so only admins can edit the group settings" ||
        text == author + " changed this group\'s settings to allow only admins to send messages to this group" ||
        text == author + " changed this group\'s settings to allow all participants to send messages to this group")
        result = MessageType::settings;
    if (text == "You're now an admin")
        result = MessageType::admin;
    if (text == "<Media omitted>" ||
        text == "image omitted" ||
        text == "video omitted" ||
        text == "sticker omitted" ||
        text == "GIF omitted" ||
        text == "audio omitted" ||
        endswith(text, "document omitted"))
        result = MessageType::media; // consider splitting into different types of media, if known? // also what about links?
    if (text == "You blocked this contact. Tap to unblock.")
        result = MessageType::block;
    if (text == "You unblocked this contact.")
        result = MessageType::unblock;
    if (text == "You deleted this message" ||
        text == "You deleted this message." ||
        text == "This message was deleted.")
        result = MessageType::deleted;
    return result;
}

void printMessage(Message* message) {
    std::cout << "Message at " << message->start << "; len=" << message->length << std::endl;
    std::cout << "Written by " << message->author << std::endl << "Time: ";
    std::cout << +message->year << '-' << +message->month << '-' << +message->day << 'T' << +message->hour << ':' << +message->minute << std::endl;
    std::cout << "ioc=" << message->ioc << std::endl;
    std::cout << "messageType=" << +(message->messageType) << std::endl;
}

bool isNewMessage(std::string line, int format) {
    if (line.length() < 20) return false;
    if (format & Parentheses) {
        if (line[0] != '[') return false;
        line.erase(0, 1);
    }
    bool flag = true;
    // /\d\d(\/|\.)\d\d(\/|\.)\d\d\d\d, /
    flag &= isdigit(line[0]) &&
            isdigit(line[1]) &&
            isSeparator(line[2], format) &&
            isdigit(line[3]) &&
            isdigit(line[4]) &&
            isSeparator(line[5], format) &&
            isdigit(line[6]) &&
            isdigit(line[7]) &&
            isdigit(line[8]) &&
            isdigit(line[9]) &&
            line[10] == ',' &&
            line[11] == ' ';
    if (line[13] == ':') {
        // insert `0` between 11 and 12, then continue normally. (singular left zero-pad)
        line.insert(12, 1, '0');
    }
    // /(\d|0)\d:\d\d/
    flag &= isdigit(line[12]) &&
            isdigit(line[13]) &&
            line[14] == ':' &&
            isdigit(line[15]) &&
            isdigit(line[16]);

    // 0123456789ABCDEF- (indexes)
    // dd.dd.dddd, 0d:dd  OR
    // dd.dd.dddd, dd:dd

    if (format & Parentheses) {
        // :dd]
        flag &= line.length() > 20 &&
                line[17] == ':' &&
                isdigit(line[18]) &&
                isdigit(line[19]) &&
                line[20] == ']';
    } else if (format & Dash) {
        flag &= line[17] == ' ' &&
                line[18] == '-' &&
                line[19] == ' ';
    } else {
        throw 33; // invalid format
    }


    return flag;

    // TODO: check number's ranges (1-12, 1-31) // look at `atoi`.
}

bool isSeparator(char c, int format) {
    if (format & Slashes) return c == '/';
    if (format & Dots) return c == '.';
    throw 33; // invalid format, cannot have neither Slashes nor Dots.
}

void removeNonPositiveChars(std::string& text) {
    // Filter out all negative characters (signed char), especially [U+200E] (Left-to-Right Mark (LRM))
    int i, j;
    for (j = -1, i = 0; text[i] != '\0'; i++) {
        if (text[i] > 0)
            text[++j] = text[i];
    }
    text[j+1] = '\0';
    text = std::string(text.c_str());
}


bool startswith(const std::string& text, const std::string& beginning) {
    if (text.length() < beginning.length()) {
        return false;  // text is shorter, cannot start with beginning
    }

    for (int i = 0; i < beginning.length(); ++i) {
        if (text[i] != beginning[i]) {
            return false;  // characters mismatch, not a match
        }
    }
    
    return true;
}

bool endswith(const std::string& text, const std::string& ending) {
    if (text.length() < ending.length()) {
        return false;  // text is shorter, cannot end with ending
    }

    int textLength = text.length();
    int endingLength = ending.length();

    for (int i = 0; i < endingLength; ++i) {
        if (text[textLength - endingLength + i] != ending[i]) {
            return false;  // characters mismatch, not a match
        }
    }

    return true;
}

bool contains(const std::string& a, const std::string& b) {
    return a.find(b) != std::string::npos;
}





// Also cotinue translating the Python code

// I've decided that it would be better to have everything as just a huge single DF
// then, I could use `struct`s for the entries,
// it'd make - calculating everything in one iteration - natural
// make those structs. Also note that an int (or a short) could be enough for authors, if you maintain a separate list thereof

// instead of saving strings (char arrays) on the HEAP or STACK or whatever, save references to places in the file.
// much more memory efficient, slight decrease in time efficiency because to print you need to read the file again.
