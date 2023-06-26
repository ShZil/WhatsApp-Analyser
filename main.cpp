#include <iostream>
#include <fstream>
#include <string>
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

int main() {
    printFile("raw/text.txt");
    std::cout << "WOOO" << std::endl;
    return 0;
}
