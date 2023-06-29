#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>
#include <sstream>

#include "txt_files.h"

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

enum MessageFormat {
    // DMY and MDY are exclusive
    DMY = 1,
    MDY = 2,
    // Slashes and dots are exclusive
    Slashes = 4,
    Dots = 8,
    // Paremtjeses and Dash are exclusive
    ParenthesesHMS = 16,
    Dash = 32
};

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
};
void printMessage(Message*);

int main() {
    std::vector<std::string> paths = getPaths("raw/");
    for (std::string path : paths)
    {
        // TODO: consider only `.txt` files, not `.txt.ignore`.
        std::cout << "Reading: " << path << std::endl; // improve this printing to call out "This is the chat with XYZ"
        
        handleFile(path);
    }

    return 0;
}

void handleFile(std::string path) {
    int messageFormat = DMY | Slashes | Dash; // later on I'll add automatic format detection. Right now it's hardcoded.

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
    // TODO: get rid of `message`?
    f.clear();
    
    char buffer[8];
    extract(f, buffer, 13); // extract 7 bytes from position 13 in the file `f`.
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
    if (format & ParenthesesHMS && content[0] == '[') {
        content.erase(0, 1);
    }
    if (content[13] == ':') {
        // insert `0` between 11 and 12, then continue normally. (singular left zero-pad)
        content.insert(11, 1, '0');
    }
    int i = 0;
    std::stringstream datetime;
    for (i = 0; i <= 16; ++i) {
        datetime << content[i];
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
    for (i = (format & ParenthesesHMS) ? 22 : 20; i <= message->length; ++i) {
        if (content[i] == ':') {
            found = true;
            break;
        }
        author << content[i];
    }
    message->author = found ? author.str() : "WhatsApp";

    // actual message content
    std::string text;
    try
    {
        text = content.substr(i + 2);
    }
    catch (const std::out_of_range& e)
    {
        text = "";
    }

    if (found) {
        // ioc = sum(count * (count-1)) where count is every character's appearance count.
        // Divided by length * (length-1)
        // equivalent to `ioc = [sum(#²)-l]/[l²-l]`, where `#` is count and `l` is length.
        // 1. Get a list of counts. Which character each count is associated with -- doesn't matter. // how would I do this efficiently?
        // 2. Calculate the sum of squares.
        // 3. Plug into `message->ioc = [sum_of_squares-l]/[l²-l]`.
    } else {
        message->ioc = 0.0f; // WhatsApp's messages are uninteresting and shouldn't affect statistics.
    }

    printMessage(message);
    std::cout << text << std::endl;

    delete message;

    // save the message to a global array.
}

void printMessage(Message* message) {
    std::cout << "Message at " << message->start << "; len=" << message->length << std::endl;
    std::cout << "Written by " << message->author << std::endl << "Time: ";
    std::cout << +message->year << '-' << +message->month << '-' << +message->day << 'T' << +message->hour << ':' << +message->minute << std::endl;
    std::cout << "ioc=" << message->ioc << std::endl;
}

bool isNewMessage(std::string line, int format) {
    if (line.length() < 20) return false;
    if (format & ParenthesesHMS) {
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
        line.insert(11, 1, '0');
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

    if (format & ParenthesesHMS) {
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

// Also cotinue translating the Python code

// I've decided that it would be better to have everything as just a huge single DF
// then, I could use `struct`s for the entries,
// it'd make - calculating everything in one iteration - natural
// make those structs. Also note that an int (or a short) could be enough for authors, if you maintain a separate list thereof

// instead of saving strings (char arrays) on the HEAP or STACK or whatever, save references to places in the file.
// much more memory efficient, slight decrease in time efficiency because to print you need to read the file again.
