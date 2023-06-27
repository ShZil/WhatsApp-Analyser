#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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
    TODO:


*/

/*
    Conceptual:

I wanna parse each message (collection of lines) and immediately pass it to DFs' analyses.
So the whole thing is just one big iteration.
*/

int main()
{
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
    std::ifstream file(path);

    while (std::getline(file, line)) {
        // Outsource to a function that determines whether a specific line is a `start` of a message or not.
        handleMessage(line); // improve the logic here, to send multiple lines through.
    }

    file.close();
}

void handleMessage(std::string message) {
    std::cout << message << std::endl; // propagate the message to all the DFs
}

// Also cotinue translating the Python code
