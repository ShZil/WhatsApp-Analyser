#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "txt_files.h"
#include <assert.h>

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
    TODO:


*/

/*
    Conceptual:

I wanna parse each message (collection of lines) and immediately pass it to DFs' analyses.
So the whole thing is just one big iteration.
*/

void handleFile(std::string);
void handleMessage(std::string, std::streampos);
bool isNewMessage(std::string);

int main() {
    std::vector<std::string> paths = getPaths("raw/");
    for (std::string path : paths)
    {
        std::cout << path << std::endl; // improve this printing to call out "This is the chat with XYZ"
        
        handleFile(path);
    }
    return 0;
}

void handleFile(std::string path) {
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
            line.pop_back();
        if (line.empty() || line.back() != '\n')
            line += "\n";
        if (isNewMessage(line)) {
            handleMessage(message, start);
            message = "";
            start = pos;
        }
        message += line;
        pos = currentPosition(f);
    }
    handleMessage(message, start);
    // get rid of `message`
    f.clear();
    
    char buffer[8];
    extract(f, buffer, 13);
    // std::cout << buffer << std::endl;

    f.close();
}

void handleMessage(std::string message, std::streampos startpos) {
    if (!message.empty() && message.back() == '\n')
        message.pop_back(); // remove trailing newline
    int length = message.length();
    int start = (int)startpos;
    std::cout << "@" << start << " len=" << length << "  " << message << std::endl;
    // propagate the message to all the DFs
}

bool isNewMessage(std::string line) {
    if (line.length() == 0) return false;
    return line.at(0) == '!'; // temporary lol
}

// Also cotinue translating the Python code

// I've decided that it would be better to have everything as just a huge single DF
// then, I could use `struct`s for the entries,
// it'd make - calculating everything in one iteration - natural
// make those structs. Also note that an int (or a short) could be enough for authors, if you maintain a separate list thereof

// instead of saving strings (char arrays) on the HEAP or STACK or whatever, save references to places in the file.
// much more memory efficient, slight decrease in time efficiency because to print you need to read the file again.
