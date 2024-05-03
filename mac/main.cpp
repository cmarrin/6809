//
//  main.cpp
//  emulator
//
//  Created by Chris Marrin on 4/23/24.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "MC6809.h"
#include "srec.h"

// Test data (see test/simple.asm
char simpleTest[ ] =    "S01C00005B6C77746F6F6C7320342E32325D2073696D706C652E61736D18\n"
                        "S1120200CC0208FD020F20F848656C6C6F5C6E31\n"
                        "S5030001FB\n"
                        "S9030000FC\n"
;

class SRecordInfo : public SRecordParser
{
  public:
    virtual  ~SRecordInfo() { }

  protected:
    virtual  bool  FinishSegment( unsigned addr, unsigned len );
    virtual  bool  Header( const SRecordHeader *sRecHdr );
    virtual  bool  StartAddress( const SRecordData *sRecData );
};

bool SRecordInfo::Header( const SRecordHeader *sRecHdr )
{
   std::cout << "Module: '" << sRecHdr->m_module  << "', ";
   std::cout << "Ver: '"    << sRecHdr->m_ver     << "', ";
   std::cout << "Rev: '"    << sRecHdr->m_rev     << "', ";
   std::cout << "Descr: '"  << sRecHdr->m_comment << "'" << std::endl;

   return true;
}

bool SRecordInfo::FinishSegment( unsigned addr, unsigned len )
{
   std::ios_base::fmtflags  svFlags = std::cout.flags();
   char                     svFill  = std::cout.fill();

   std::cout << "Segment Address: 0x"
             << std::setbase( 16 ) << std::setw( 6 ) << std::setfill( '0' ) << addr
             << " Len: 0x" 
             << std::setbase( 16 ) << std::setw( 6 ) << std::setfill( '0' ) << len
             << std::endl;

   std::cout.flags( svFlags );
   std::cout.fill( svFill );

   return true;
}

bool SRecordInfo::StartAddress( const SRecordData *sRecData )
{
   std::ios_base::fmtflags  svFlags = std::cout.flags();
   char                     svFill  = std::cout.fill();

   std::cout << "  Start Address: 0x"
             << std::setbase( 16 ) << std::setw( 6 ) << std::setfill( '0' ) << sRecData->m_addr
             << std::endl;

   std::cout.flags( svFlags );
   std::cout.fill( svFill );

   return true;
}

int main(int argc, char * const argv[])
{
    mc6809::Emulator emu(65536);
        
    if (argc < 2) {
        // use sample
        emu.load(simpleTest);
    } else {
        SRecordInfo sRecInfo;
        sRecInfo.ParseFile( argv[ 1 ]);
        sRecInfo.Flush();

//        std::ifstream f(argv[1]);
//        if (f.is_open()) {
//            std::stringstream buffer;
//            buffer << f.rdbuf();
//            f.close();
//            
//            emu.load(buffer.str().c_str());
//        } else {
//            std::cout << "Unable to open file";
//            return -1;
//        }
    }
    
    emu.execute(0x200);
    return 0;
}
