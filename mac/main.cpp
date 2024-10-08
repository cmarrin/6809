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
#include <unistd.h>
#include <sys/ioctl.h>

#include "BOSS9.h"
#include "Format.h"

// Test data
char simpleTest[ ] =
    "S02000005B6C77746F6F6C7320342E32325D2048656C6C6F576F726C642E61736DA2\n"
    "S1130200860AB702298E0219BDFC02860ABDFC00CB\n"
    "S11302107A02292EF04FBDFC0E48656C6C6F206687\n"
    "S10C0220726F6D2036383039008C\n"
    "S5030003F9\n"
    "S9030200FA\n"
;

static constexpr uint32_t MemorySize = 65536;

class MacBOSS9 : public mc6809::BOSS9<MemorySize>
{
  public:
    MacBOSS9() : BOSS9()
    {
        _echoBS = false;
    }
    
    virtual ~MacBOSS9() { }

  protected:
    virtual void putc(char c) const override
    {
        // Throttle character output so console doesn't get swamped
        usleep(100);
        fputc(c, stdout);
    }
    
    virtual int getc() override
    {
        int bytes = 0;
        if (ioctl(0, FIONREAD, &bytes) == -1) {
            return -1;
        }
        if (bytes == 0) {
            return 0;
        }
        return getchar();
    }

    virtual bool handleRunLoop() override
    {
        return true;
    }
    
  private:
    uint32_t _cursor = 0;
};

char* findNextLine(char* s)
{
    while (*s != '\n' && *s != '\0') {
        ++s;
    }
    if (*s != '\0') {
        s++;
    }
    return s;
}

//
// Usage: emulator -m [filename]
//
//          -m:         stop in monitor on entry
//          filename:   s19 file to load. If none given a simple test progam is loaded
int main(int argc, char * const argv[])
{
    // For now we're going to assume 64KB of RAM and that there will
    // Be some sort of system functions at $E000, which is where the
    // ACIA is located in some SBC systems. So we'll out the stack
    // at $E000
    //
    // We'll figure out the rest later.
    
    system("stty raw");
    
    MacBOSS9 boss9;
    
    boss9.emulator().setStack(0xe000);
    
    uint16_t startAddr = 0;
    bool startInMonitor = false;
    int c;
        
    while ((c = getopt(argc, argv, "m")) != -1) {
        switch (c) {
            case 'm':
                startInMonitor = true;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-m] [filename]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    char* fileString = nullptr;
    bool isFileStringAllocated = false;
    uint32_t size = 0;
    
    if (optind >= argc) {
        // use sample
        fileString = simpleTest;
        size = sizeof(simpleTest);
    } else {
        std::string filename = argv[optind];

        std::ifstream stream;
        stream.open(filename.c_str(), std::fstream::in);
        if (stream.fail()) {
            std::cout << "Can't open '" << filename << "'\n";
            return -1;
        }

        // Are we compiling?
        std::string path = filename.substr(0, filename.find_last_of('.'));
        std::string suffix = filename.substr(filename.find_last_of(".") + 1);
        
        if (suffix == "clvr") {
            // Compile the clover file
            std::string cmd = "/usr/local/bin/clvr -9 ";
            cmd += filename;
            int retval = std::system(cmd.c_str());
            if (WIFEXITED(retval) == 0) {
                std::cout << "Compile of '" << filename << "' abnormally terminated, exiting\n";
                return -1;
            }
            if (WEXITSTATUS(retval) != 0) {
                std::cout << "Compile of '" << filename << "' failed, exiting\n";
                return -1;
            }
            
            // Successful compile the .asm file is in the same dir, assemble it
            cmd = "/usr/local/bin/lwasm -I emulator -f srec -o";
            cmd += path;
            cmd += ".s19 -l";
            cmd += path;
            cmd += ".lst ";
            cmd += path;
            cmd += ".asm";
            retval = std::system(cmd.c_str());
            if (WIFEXITED(retval) == 0) {
                std::cout << "Assembly of '" << path << ".asm' abnormally terminated, exiting\n";
                return -1;
            }
            if (WEXITSTATUS(retval) != 0) {
                std::cout << "Assembly of '" << path << ".asm' failed, exiting\n";
            }
            
            filename = path + ".s19";
        }

        std::ifstream f(filename);
        if (f.is_open()) {
            size = uint32_t(std::filesystem::file_size(filename));
            fileString = new char[size + 1];
            f.read(fileString, size);
            fileString[size] = '\0';
            f.close();
        } else {
            std::cout << "Unable to open '" << filename << "'\n";
            return -1;
        }
    }
    
    boss9.emulator().loadStart();
    while (true) {
        bool finished;
        if (!boss9.emulator().loadLine(fileString, finished)) {
            std::cout << "Unable to load file\n";
            return -1;
        }

        fileString = findNextLine(fileString);
        if (*fileString == '\0') {
            break;
        }
    }
    
    startAddr = boss9.emulator().loadEnd();
    
    if (isFileStringAllocated) {
        delete [ ] fileString;
    }

    boss9.startExecution(startAddr, startInMonitor);
    
    while (boss9.continueExecution()) { }
    
    if (boss9.emulator().error() != mc6809::Emulator::Error::None) {
        fmt::printf("*** finished with error: %d\n", int32_t(boss9.emulator().error()));
    } else {
        fmt::printf("    finished successfully\n");
    }
    return 0;
}
