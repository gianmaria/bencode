#include <fcntl.h>
#include <io.h>

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <filesystem>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::string_view;
using std::ifstream;
using std::ofstream;
using std::vector;

namespace fs = std::filesystem;

#include "win_cpp_crypt.h"

int base64_encode(const string& input_file)
{
    ifstream ifs(input_file, std::ios::binary);
    if (!ifs)
    {
        cerr << "Error: cannot open input file '" << input_file << "'" << endl;
        return 1;
    }

    auto input_file_size = fs::file_size(input_file);

    vector<char> input_file_content(input_file_size);

    ifs.read(input_file_content.data(), input_file_content.size());

    if (not ifs.good())
    {
        cerr << "Error: only " << ifs.gcount() << " bytes could be read from " << input_file << endl;
        return 1;
    }

    auto base64 = WinCppCrypt::Util::base64Encode(
        reinterpret_cast<BYTE*>(input_file_content.data()),
        static_cast<DWORD>(input_file_content.size())
    );

    if (base64.hasError())
    {
        cerr << "Error: " << base64.error().what() << endl;
        return 1;
    }

    cout << WinCppCrypt::Util::toSv(base64.unwrap());

    return 0;
}

int base64_decode(const string& input_file)
{
    ifstream ifs(input_file, std::ios::binary);
    if (!ifs)
    {
        cerr << "Error: cannot open input file '" << input_file << "'" << endl;
        return 1;
    }

    auto input_file_size = fs::file_size(input_file);

    vector<char> input_file_content(input_file_size);

    ifs.read(input_file_content.data(), input_file_content.size());

    if (not ifs.good())
    {
        cerr << "Error: only " << ifs.gcount() << " bytes could be read from " << input_file << endl;
        return 1;
    }

    auto base64 = WinCppCrypt::Util::base64Decode(
        reinterpret_cast<LPCSTR>(input_file_content.data()),
        static_cast<DWORD>(input_file_content.size())
    );

    if (base64.hasError())
    {
        cerr << "Error: " << base64.error().what() << endl;
        return 1;
    }

    cout.flush();
    int prev_mode = _setmode(_fileno(stdout), _O_BINARY);

    if (prev_mode == -1)
    {
        cerr << "Error: cannot set stdout to binary mode" << endl;
        return 1;
    }

    if (prev_mode != _O_TEXT)
    {
        cerr << "Warning: stdout was not in text mode" << endl;
    }

    cout.write(reinterpret_cast<const char*>(base64.unwrap().data()), base64.unwrap().size());

    return 0;
}


int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            cerr << "Usage:" << endl
                << "    bencode [-e|-d] <input_file>" << endl;

            return 1;
        }

        string_view mode = argv[1];

        if (not (mode == "-e" || mode == "-d"))
        {
            cerr << "Error: invalid mode '" << mode << "'" << endl;
            return 1;
        }

        if (mode == "-e")
        {
            return base64_encode(argv[2]);
        }
        else
        {
            return base64_decode(argv[2]);
        }

    }
    catch (const std::exception& e)
    {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
