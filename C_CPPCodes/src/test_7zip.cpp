#include <iostream>
#include <vector>
#include <string>
#include <bit7z/bit7z.hpp>


void test_func(){
    try {
        using namespace bit7z;
        Bit7zLibrary lib{ "7z.dll" };
        BitFileCompressor compressor{ lib, BitFormat::Zip };

        std::vector< std::string > files = { 
            R"(C:\Users\XiaoFei\Pictures\Screenshots\pytest_effect_updated.jpg)", 
            R"(C:\MCodes\C_CPPCodes\temp\bit7z\BUILD.txt)"
            };
        compressor.compress( files, "output_archive.zip" );
        
        BitFileExtractor extractor{ lib, BitFormat::SevenZip };
        extractor.extract( R"(C:\MCodes\C_CPPCodes\temp\7z2405-extra.7z)", "out/7z2405-extra" );

    }catch ( const bit7z::BitException& ex){
        /* Do something with ex.what()...*/
        std::cout<< ex.what() <<std::endl;
     }


}

int main(int argc, char *argv[])
{
    
    test_func();
    return 0; //
}