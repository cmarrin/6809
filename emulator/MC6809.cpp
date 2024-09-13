/*-------------------------------------------------------------------------
    This source file is a part of the MC6809 Simulator
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2024, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/
//
//  MC6809.cpp
//  6809 simulator
//
//  Created by Chris Marrin on 4/22/24.
//

#include "MC6809.h"
#include "BOSS9.h"

using namespace mc6809;

static_assert (sizeof(Opcode) == 3, "Opcode is wrong size");

static inline const char* opToString(Op op)
{
    switch (op) {
        default: return "???";
        
        case Op::ILL    : return "ILL";
        case Op::ABX    : return "ABX";
        case Op::ADC    : return "ADC";
        case Op::ADD8   :
        case Op::ADD16  : return "ADD";
        case Op::AND    : return "AND";
        case Op::ANDCC  : return "ANDCC";
        case Op::ASL    : return "ASL";
        case Op::ASR    : return "ASR";
        case Op::BCC    : return "BCC";
        case Op::BCS    : return "BCS"; 
        case Op::BEQ    : return "BEQ";
        case Op::BGE    : return "BGE";
        case Op::BGT    : return "BGT";
        case Op::BHI    : return "BHI";
        case Op::BHS    : return "BHS";
        case Op::BIT    : return "BIT";
        case Op::BLE    : return "BLE";
        case Op::BLO    : return "BLO"; 
        case Op::BLS    : return "BLS";
        case Op::BLT    : return "BLT";
        case Op::BMI    : return "BMI";
        case Op::BNE    : return "BNE";
        case Op::BPL    : return "BPL";
        case Op::BRA    : return "BRA";
        case Op::BRN    : return "BRN";
        case Op::BSR    : return "BSR"; 
        case Op::BVC    : return "BVC";
        case Op::BVS    : return "BVS";
        case Op::CLR    : return "CLR";
        case Op::CMP8   :
        case Op::CMP16  : return "CMP";
        case Op::COM    : return "COM";
        case Op::CWAI   : return "CWAI";
        case Op::DAA    : return "DAA";
        case Op::DEC    : return "DEC"; 
        case Op::EOR    : return "EOR";
        case Op::EXG    : return "EXG";
        case Op::INC    : return "INC";
        case Op::JMP    : return "JMP";
        case Op::JSR    : return "JSR";
        case Op::LD8    :
        case Op::LD16   : return "LD";
        case Op::LEA    : return "LEA"; 
        case Op::LSR    : return "LSR";
        case Op::MUL    : return "MUL";
        case Op::NEG    : return "NEG";
        case Op::NOP    : return "NOP";
        case Op::OR     : return "OR";
        case Op::ORCC   : return "ORCC";
        case Op::PSH    : return "PSH";
        case Op::PUL    : return "PUL";
        case Op::ROL    : return "ROL"; 
        case Op::ROR    : return "ROR";
        case Op::RTI    : return "RTI";
        case Op::RTS    : return "RTS";
        case Op::SBC    : return "SBC";
        case Op::SEX    : return "SEX";
        case Op::ST8    :
        case Op::ST16   : return "ST";
        case Op::SUB8   :
        case Op::SUB16  : return "SUB";
        case Op::SWI    : return "SWI";
        case Op::SYNC   : return "SYNC";
        case Op::TFR    : return "TFR";
        case Op::TST    : return "TST"; 
        case Op::FIRQ   : return "FIRQ"; 
        case Op::IRQ    : return "IRQ"; 
        case Op::NMI    : return "NMI"; 
        case Op::RESTART: return "RESTART";
    }
}

static inline const char* regToString(Reg op, Op prevOp)
{
    switch (op) {
        default:        return "";
        case Reg::A:    return "A";
        case Reg::B:    return "B";
        case Reg::D:    return "D";
        case Reg::X:    return "X";
        case Reg::Y:    return "Y";
        case Reg::U:    return "U";
        case Reg::S:    return "S";
        case Reg::CC:   return "CC";
        case Reg::PC:   return "PC";
        case Reg::DP:   return "DP";
        case Reg::DDU:  return (prevOp == Op::Page2) ? "D" : ((prevOp == Op::Page3) ? "U" : "D");
        case Reg::XYS:  return (prevOp == Op::Page2) ? "Y" : ((prevOp == Op::Page3) ? "S" : "X");
        case Reg::XY:   return (prevOp == Op::Page2) ? "Y" : ((prevOp == Op::Page3) ?  "" : "X");
        case Reg::US:   return (prevOp == Op::Page2) ? "S" : ((prevOp == Op::Page3) ?  "" : "U");
    }
}

static constexpr Opcode opcodeTable[ ] = {
    /*00*/  	{ Op::NEG	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	},
    /*01*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*02*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*03*/  	{ Op::COM	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	},
    /*04*/  	{ Op::LSR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	},
    /*05*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*06*/  	{ Op::ROR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	},
    /*07*/  	{ Op::ASR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	},
    /*08*/  	{ Op::ASL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	},
    /*09*/  	{ Op::ROL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	},
    /*0A*/  	{ Op::DEC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	},
    /*0B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*0C*/  	{ Op::INC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	},
    /*0D*/  	{ Op::TST	  , Reg::M8   , Left::Ld  , Right::None , Adr::Direct	},
    /*0E*/  	{ Op::JMP	  , Reg::None , Left::None, Right::None , Adr::Direct	},
    /*0F*/  	{ Op::CLR	  , Reg::M8   , Left::St  , Right::None  , Adr::Direct	},
    /*10*/  	{ Op::Page2	  , Reg::None , Left::None, Right::None , Adr::None	    },
    /*11*/  	{ Op::Page3	  , Reg::None , Left::None, Right::None , Adr::None	    },
    /*12*/  	{ Op::NOP	  , Reg::None , Left::None, Right::None , Adr::Inherent },
    /*13*/  	{ Op::SYNC	  , Reg::None , Left::None, Right::None , Adr::Inherent	},
    /*14*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*15*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*16*/  	{ Op::BRA	  , Reg::None , Left::None, Right::None , Adr::RelL	    },
    /*17*/  	{ Op::BSR	  , Reg::None , Left::None, Right::None , Adr::RelL	    },
    /*18*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*19*/  	{ Op::DAA	  , Reg::None , Left::None, Right::None , Adr::Inherent	},
    /*1A*/  	{ Op::ORCC	  , Reg::None , Left::None, Right::None , Adr::Immed8	},
    /*1B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*1C*/  	{ Op::ANDCC	  , Reg::None , Left::None, Right::None , Adr::Immed8	},
    /*1D*/  	{ Op::SEX	  , Reg::None , Left::None, Right::None , Adr::Inherent	},
    /*1E*/  	{ Op::EXG	  , Reg::None , Left::None, Right::None , Adr::Immed8	},
    /*1F*/  	{ Op::TFR	  , Reg::None , Left::None, Right::None , Adr::Immed8	},
    /*20*/  	{ Op::BRA	  , Reg::None , Left::None, Right::None , Adr::Rel	    },
    /*21*/  	{ Op::BRN	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*22*/  	{ Op::BHI	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*23*/  	{ Op::BLS	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*24*/  	{ Op::BHS	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*25*/  	{ Op::BLO	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*26*/  	{ Op::BNE	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*27*/  	{ Op::BEQ	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*28*/  	{ Op::BVC	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*29*/  	{ Op::BVS	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*2A*/  	{ Op::BPL	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*2B*/  	{ Op::BMI	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*2C*/  	{ Op::BGE	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*2D*/  	{ Op::BLT	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*2E*/  	{ Op::BGT	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*2F*/  	{ Op::BLE	  , Reg::None , Left::None, Right::None , Adr::RelP	    },
    /*30*/  	{ Op::LEA	  , Reg::X    , Left::St  , Right::None , Adr::Indexed	},
    /*31*/  	{ Op::LEA	  , Reg::Y    , Left::None, Right::None , Adr::Indexed	},
    /*32*/  	{ Op::LEA	  , Reg::S    , Left::None, Right::None , Adr::Indexed	},
    /*33*/  	{ Op::LEA	  , Reg::U    , Left::None, Right::None , Adr::Indexed	},
    /*34*/  	{ Op::PSH	  , Reg::S    , Left::None, Right::None , Adr::Immed8	},
    /*35*/  	{ Op::PUL	  , Reg::S    , Left::None, Right::None , Adr::Immed8	},
    /*36*/  	{ Op::PSH	  , Reg::U    , Left::None, Right::None , Adr::Immed8	},
    /*37*/  	{ Op::PUL	  , Reg::U    , Left::None, Right::None , Adr::Immed8	},
    /*38*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    },
    /*39*/  	{ Op::RTS	  , Reg::None , Left::None, Right::None , Adr::Inherent	},
    /*3A*/  	{ Op::ABX	  , Reg::None , Left::None, Right::None , Adr::Inherent	},
    /*3B*/  	{ Op::RTI	  , Reg::None , Left::None, Right::None , Adr::Inherent	},          // 6 if FIRQ, 15 if IRQ
    /*3C*/  	{ Op::CWAI	  , Reg::None , Left::None, Right::None , Adr::Inherent	},
    /*3D*/  	{ Op::MUL	  , Reg::None , Left::None, Right::None , Adr::Inherent	},
    /*3E*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    },
    /*3F*/  	{ Op::SWI	  , Reg::None , Left::None, Right::None , Adr::Inherent	},
    /*40*/  	{ Op::NEG	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	},
    /*41*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    },
    /*42*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*43*/  	{ Op::COM	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	},
    /*44*/  	{ Op::LSR	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	},
    /*45*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*46*/  	{ Op::ROR	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	},
    /*47*/  	{ Op::ASR	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	},
    /*48*/  	{ Op::ASL	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	},
    /*49*/  	{ Op::ROL	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	},
    /*4A*/  	{ Op::DEC	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	},
    /*4B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*4C*/  	{ Op::INC	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	},
    /*4D*/  	{ Op::TST	  , Reg::A    , Left::Ld  , Right::None , Adr::Inherent	},
    /*4E*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    },
    /*4F*/  	{ Op::CLR	  , Reg::A    , Left::St  , Right::None , Adr::Inherent	},
    /*50*/  	{ Op::NEG	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	},
    /*51*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    },
    /*52*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*53*/  	{ Op::COM	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	},
    /*54*/  	{ Op::LSR	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	},
    /*55*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*56*/  	{ Op::ROR	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	},
    /*57*/  	{ Op::ASR	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	},
    /*58*/  	{ Op::ASL	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	},
    /*59*/  	{ Op::ROL	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	},
    /*5A*/  	{ Op::DEC	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	},
    /*5B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*5C*/  	{ Op::INC	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	},
    /*5D*/  	{ Op::TST	  , Reg::B    , Left::Ld  , Right::None , Adr::Inherent	},
    /*5E*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    },
    /*5F*/  	{ Op::CLR	  , Reg::B    , Left::St  , Right::None , Adr::Inherent	},
    /*60*/  	{ Op::NEG	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	},
    /*61*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    },
    /*62*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*63*/  	{ Op::COM	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	},
    /*64*/  	{ Op::LSR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	},
    /*65*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*66*/  	{ Op::ROR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	},
    /*67*/  	{ Op::ASR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	},
    /*68*/  	{ Op::ASL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	},
    /*69*/  	{ Op::ROL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	},
    /*6A*/  	{ Op::DEC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	},
    /*6B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*6C*/  	{ Op::INC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	},
    /*6D*/  	{ Op::TST	  , Reg::M8   , Left::Ld  , Right::None , Adr::Indexed	},
    /*6E*/  	{ Op::JMP	  , Reg::None , Left::None, Right::None , Adr::Indexed	},
    /*6F*/  	{ Op::CLR	  , Reg::M8   , Left::St  , Right::None , Adr::Indexed	},
    /*70*/  	{ Op::NEG	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	},
    /*71*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    },
    /*72*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*73*/  	{ Op::COM	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	},
    /*74*/  	{ Op::LSR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	},
    /*75*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*76*/  	{ Op::ROR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	},
    /*77*/  	{ Op::ASR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	},
    /*78*/  	{ Op::ASL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	},
    /*79*/  	{ Op::ROL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	},
    /*7A*/  	{ Op::DEC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	},
    /*7B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*7C*/  	{ Op::INC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	},
    /*7D*/  	{ Op::TST	  , Reg::M8   , Left::Ld  , Right::None , Adr::Extended	},
    /*7E*/  	{ Op::JMP	  , Reg::None , Left::None, Right::None , Adr::Extended	},
    /*7F*/  	{ Op::CLR	  , Reg::M8   , Left::St  , Right::None , Adr::Extended	},
    /*80*/  	{ Op::SUB8	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	},
    /*81*/  	{ Op::CMP8	  , Reg::A    , Left::Ld  , Right::None , Adr::Immed8	},
    /*82*/  	{ Op::SBC	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	},
    /*83*/  	{ Op::SUB16	  , Reg::DDU  , Left::Ld  , Right::None , Adr::Immed16	},
    /*84*/  	{ Op::AND	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	},
    /*85*/  	{ Op::BIT	  , Reg::A    , Left::Ld  , Right::None , Adr::Immed8	},
    /*86*/  	{ Op::LD8	  , Reg::A    , Left::St  , Right::None , Adr::Immed8	},
    /*87*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*88*/  	{ Op::EOR	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	},
    /*89*/  	{ Op::ADC	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	},
    /*8A*/  	{ Op::OR	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	},
    /*8B*/  	{ Op::ADD8	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	},
    /*8C*/  	{ Op::CMP16	  , Reg::XYS  , Left::Ld  , Right::None , Adr::Immed16	},
    /*8D*/  	{ Op::BSR	  , Reg::None , Left::None, Right::None , Adr::Rel      },
    /*8E*/  	{ Op::LD16	  , Reg::XY   , Left::St  , Right::None , Adr::Immed16	},
    /*8F*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*90*/  	{ Op::SUB8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*91*/  	{ Op::CMP8	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Direct	},
    /*92*/  	{ Op::SBC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*93*/  	{ Op::SUB16	  , Reg::DDU  , Left::Ld  , Right::Ld16 , Adr::Direct	},
    /*94*/  	{ Op::AND	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*95*/  	{ Op::BIT	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Direct	},
    /*96*/  	{ Op::LD8	  , Reg::A    , Left::St  , Right::Ld8  , Adr::Direct	},
    /*97*/  	{ Op::ST8	  , Reg::A    , Left::Ld  , Right::St8  , Adr::Direct	},
    /*98*/  	{ Op::EOR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*99*/  	{ Op::ADC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*9A*/  	{ Op::OR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*9B*/  	{ Op::ADD8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*9C*/  	{ Op::CMP16	  , Reg::X    , Left::Ld  , Right::Ld8  , Adr::Direct	},
    /*9D*/  	{ Op::JSR	  , Reg::None , Left::None, Right::None , Adr::Direct	},
    /*9E*/  	{ Op::LD16	  , Reg::XY   , Left::St  , Right::Ld16 , Adr::Direct	},
    /*9F*/  	{ Op::ST16	  , Reg::XY   , Left::Ld  , Right::St16 , Adr::Direct	},
    /*A0*/  	{ Op::SUB8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*A1*/  	{ Op::CMP8	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Indexed	},
    /*A2*/  	{ Op::SBC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*A3*/  	{ Op::SUB16	  , Reg::DDU  , Left::Ld  , Right::Ld16 , Adr::Indexed	},
    /*A4*/  	{ Op::AND	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*A5*/  	{ Op::BIT	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Indexed	},
    /*A6*/  	{ Op::LD8	  , Reg::A    , Left::St  , Right::Ld8  , Adr::Indexed	},
    /*A7*/  	{ Op::ST8	  , Reg::A    , Left::Ld  , Right::St8  , Adr::Indexed	},
    /*A8*/  	{ Op::EOR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*A9*/  	{ Op::ADC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*AA*/  	{ Op::OR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*AB*/  	{ Op::ADD8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*AC*/  	{ Op::CMP16	  , Reg::XYS  , Left::Ld  , Right::Ld16 , Adr::Indexed	},
    /*AD*/  	{ Op::JSR	  , Reg::None , Left::None, Right::None , Adr::Indexed	},
    /*AE*/  	{ Op::LD16	  , Reg::XY   , Left::St  , Right::Ld16 , Adr::Indexed	},
    /*AF*/  	{ Op::ST16	  , Reg::XY   , Left::Ld  , Right::St16 , Adr::Indexed	},
    /*B0*/  	{ Op::SUB8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	},
    /*B1*/  	{ Op::CMP8	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Extended	},
    /*B2*/  	{ Op::SBC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	},
    /*B3*/  	{ Op::SUB16	  , Reg::DDU  , Left::Ld  , Right::Ld16 , Adr::Extended	},
    /*B4*/  	{ Op::AND	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	},
    /*B5*/  	{ Op::BIT	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Extended	},
    /*B6*/  	{ Op::LD8	  , Reg::A    , Left::St  , Right::Ld8  , Adr::Extended	},
    /*B7*/  	{ Op::ST8	  , Reg::A    , Left::Ld  , Right::St8  , Adr::Extended	},
    /*B8*/  	{ Op::EOR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	},
    /*B9*/  	{ Op::ADC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	},
    /*BA*/  	{ Op::OR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	},
    /*BB*/  	{ Op::ADD8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	},
    /*BC*/  	{ Op::CMP16	  , Reg::XYS  , Left::Ld  , Right::Ld16 , Adr::Extended	},
    /*BD*/  	{ Op::JSR	  , Reg::None , Left::None, Right::None , Adr::Extended	},
    /*BE*/  	{ Op::LD16	  , Reg::XY   , Left::St  , Right::Ld16 , Adr::Extended	},
    /*BF*/  	{ Op::ST16	  , Reg::XY   , Left::Ld  , Right::St16 , Adr::Extended	},
    /*C0*/  	{ Op::SUB8	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	},
    /*C1*/  	{ Op::CMP8	  , Reg::B    , Left::Ld  , Right::None , Adr::Immed8	},
    /*C2*/  	{ Op::SBC	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	},
    /*C3*/  	{ Op::ADD16	  , Reg::D    , Left::LdSt, Right::None , Adr::Immed16	},
    /*C4*/  	{ Op::AND	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	},
    /*C5*/  	{ Op::BIT	  , Reg::B    , Left::Ld  , Right::None , Adr::Immed8	},
    /*C6*/  	{ Op::LD8	  , Reg::B    , Left::St  , Right::None , Adr::Immed8	},
    /*C7*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*C8*/  	{ Op::EOR	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	},
    /*C9*/  	{ Op::ADC	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	},
    /*CA*/  	{ Op::OR	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	},
    /*CB*/  	{ Op::ADD8	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	},
    /*CC*/  	{ Op::LD16	  , Reg::D    , Left::St  , Right::None , Adr::Immed16	},
    /*CD*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*CE*/  	{ Op::LD16	  , Reg::US   , Left::St  , Right::None , Adr::Immed16	},
    /*CF*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     },
    /*D0*/  	{ Op::SUB8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*D1*/  	{ Op::CMP8	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Direct	},
    /*D2*/  	{ Op::SBC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*D3*/  	{ Op::ADD16	  , Reg::D    , Left::LdSt, Right::Ld16 , Adr::Direct	},
    /*D4*/  	{ Op::AND	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*D5*/  	{ Op::BIT	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Direct	},
    /*D6*/  	{ Op::LD8	  , Reg::B    , Left::St  , Right::Ld8  , Adr::Direct	},
    /*D7*/  	{ Op::ST8	  , Reg::B    , Left::Ld  , Right::St8  , Adr::Direct	},
    /*D8*/  	{ Op::EOR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*D9*/  	{ Op::ADC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*DA*/  	{ Op::OR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*DB*/  	{ Op::ADD8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	},
    /*DC*/  	{ Op::LD16	  , Reg::D    , Left::St  , Right::Ld16 , Adr::Direct	},
    /*DD*/  	{ Op::ST16	  , Reg::D    , Left::Ld  , Right::St16 , Adr::Direct	},
    /*DE*/  	{ Op::LD16	  , Reg::US   , Left::St  , Right::Ld16 , Adr::Direct	},
    /*DF*/  	{ Op::ST16	  , Reg::US   , Left::Ld  , Right::St16 , Adr::Direct	},
    /*E0*/  	{ Op::SUB8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*E1*/  	{ Op::CMP8	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Indexed	},
    /*E2*/  	{ Op::SBC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*E3*/  	{ Op::ADD16	  , Reg::D    , Left::LdSt, Right::Ld16 , Adr::Indexed	},
    /*E4*/  	{ Op::AND	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*E5*/  	{ Op::BIT	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Indexed	},
    /*E6*/  	{ Op::LD8	  , Reg::B    , Left::St  , Right::Ld8  , Adr::Indexed	},
    /*E7*/  	{ Op::ST8	  , Reg::B    , Left::Ld  , Right::St8  , Adr::Indexed	},
    /*E8*/  	{ Op::EOR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*E9*/  	{ Op::ADC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*EA*/  	{ Op::OR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*EB*/  	{ Op::ADD8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	},
    /*EC*/  	{ Op::LD16	  , Reg::D    , Left::St  , Right::Ld16 , Adr::Indexed	},
    /*ED*/  	{ Op::ST16	  , Reg::D    , Left::Ld  , Right::St16 , Adr::Indexed	},
    /*EE*/  	{ Op::LD16	  , Reg::US   , Left::St  , Right::Ld16 , Adr::Indexed	},
    /*EF*/  	{ Op::ST16	  , Reg::US   , Left::Ld  , Right::St16 , Adr::Indexed	},
    /*F0*/  	{ Op::SUB8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended },
    /*F1*/  	{ Op::CMP8	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Extended },
    /*F2*/  	{ Op::SBC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended },
    /*F3*/  	{ Op::ADD16	  , Reg::D    , Left::LdSt, Right::Ld16 , Adr::Extended },
    /*F4*/  	{ Op::AND	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended },
    /*F5*/  	{ Op::BIT	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Extended },
    /*F6*/  	{ Op::LD8	  , Reg::B    , Left::St  , Right::Ld8  , Adr::Extended },
    /*F7*/  	{ Op::ST8	  , Reg::B    , Left::Ld  , Right::St8  , Adr::Extended },
    /*F8*/  	{ Op::EOR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended },
    /*F9*/  	{ Op::ADC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended },
    /*FA*/  	{ Op::OR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended },
    /*FB*/  	{ Op::ADD8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended },
    /*FC*/  	{ Op::LD16	  , Reg::D    , Left::St  , Right::Ld16 , Adr::Extended },
    /*FD*/  	{ Op::ST16	  , Reg::D    , Left::Ld  , Right::St16 , Adr::Extended },
    /*FE*/  	{ Op::LD16	  , Reg::US   , Left::St  , Right::Ld16 , Adr::Extended },
    /*FF*/  	{ Op::ST16	  , Reg::US   , Left::Ld  , Right::St16 , Adr::Extended },
};

#ifdef COMPUTE_CYCLES
static constexpr uint8_t cyclesTable[ ] = {
    /*00*/  	6 , 0 , 0 , 6 , 6 , 0 , 6 , 6 , 6 , 6 , 6 , 0 , 6 , 6 , 3 , 6 ,
    /*10*/  	0 , 0 , 2 , 4 , 0 , 0 , 5 , 9 , 0 , 2 , 3 , 0 , 3 , 2 , 8 , 6 ,
    /*20*/  	3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 ,
    /*30*/  	4 , 4 , 4 , 4 , 5 , 5 , 5 , 5 , 0 , 5 , 3 , 6 , 20, 11, 0 , 19,
    /*40*/  	2 , 0 , 0 , 2 , 2 , 0 , 2 , 2 , 2 , 2 , 2 , 0 , 2 , 2 , 0 , 2 ,
    /*50*/  	2 , 0 , 0 , 2 , 2 , 0 , 2 , 2 , 2 , 2 , 2 , 0 , 2 , 2 , 0 , 2 ,
    /*60*/  	6 , 0 , 0 , 6 , 6 , 0 , 6 , 6 , 6 , 6 , 6 , 0 , 6 , 6 , 3 , 6 ,
    /*70*/  	7 , 0 , 0 , 7 , 7 , 0 , 7 , 7 , 7 , 7 , 7 , 0 , 7 , 7 , 4 , 7 ,
    /*80*/  	2 , 2 , 2 , 4 , 2 , 2 , 2 , 0 , 2 , 2 , 2 , 2 , 4 , 7 , 3 , 0 ,
    /*90*/  	4 , 4 , 4 , 6 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 6 , 7 , 5 , 5 ,
    /*A0*/  	4 , 4 , 4 , 6 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 6 , 7 , 5 , 5 ,
    /*B0*/  	5 , 5 , 5 , 7 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 7 , 8 , 6 , 6 ,
    /*C0*/  	2 , 2 , 2 , 4 , 2 , 2 , 2 , 0 , 2 , 2 , 2 , 2 , 3 , 0 , 3 , 0 ,
    /*D0*/  	4 , 4 , 4 , 6 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 5 , 5 , 5 , 5 ,
    /*E0*/  	4 , 4 , 4 , 6 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 5 , 5 , 5 , 5 ,
    /*F0*/  	5 , 5 , 5 , 7 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 6 , 6 , 6 , 6 ,
};
#endif

// Page2 & Page3
//
// All the 16 bit ops in Page2 and Page3 have the same addr modes as the
// corresponding opcode in the first page. All the auxiliary cmp/sub ops
// go with those in the base page. The same is true for all the LD16
// and ST16 ops. So there are really only 3 ops that need to deal with
// Page2 and Page3
//
// Some of the SUB16 ops have CMP16 ops in Page2 and Page3. So in the opcode table only
// have the left operand be Left::Ld, so it doesn't get stored when we use the CMP16
// variants. That means in the SUB16 handler we have to store the value. We do this
// by calling setReg(opcode->reg, _result);
//
// 83 SUBD -> 1083 CMPD  -> 1183 CMPU       * SUB16/CMP16
// 8C CMPX -> 108C CMPY  -> 118C CMPS       * SUB16/CMP16
// 8E LDX  -> 108E LDY   ->
// 93 SUBD -> 1093 CMPD  -> 1193 CMPU       * SUB16/CMP16
// 9C CMPX -> 109C CMPY  -> 119C CMPS       * SUB16/CMP16
// 9E LDX  -> 109E LDY   ->
// 9F STX  -> 109F STY   ->
// A3 SUBD -> 10A3 CMPD  -> 11A3 CMPU       * SUB16/CMP16
// AC CMPX -> 10AC CMPY  -> 11AC CMPS       * SUB16/CMP16
// AE LDX  -> 10AE LDY   ->
// AF STX  -> 10AF STY   ->
// B3 SUBD -> 10B3 CMPD  -> 11B3 CMPU       * SUB16/CMP16
// BC CMPX -> 10BC CMPY  -> 11BC CMPS       * SUB16/CMP16
// BE LDX  -> 10BE LDY   ->
// BF STX  -> 10BF STY   ->
// CE LDU  -> 10CE LDS   ->
// DE LDU  -> 10DE LDS   ->
// DF STU  -> 10DF STS   ->
// EE LDU  -> 10EE LDS   ->
// EF STU  -> 10EF STS   ->
// FE LDU  -> 10FE LDS   -> 
// FF STU  -> 10FF STS   ->

static_assert (sizeof(opcodeTable) == 256 * sizeof(Opcode), "Opcode table is wrong size");

static inline uint16_t concat(uint8_t a, uint8_t b)
{
    return (uint16_t(a) << 8) | uint16_t(b);
}

void SRecordInfo::ParseError(unsigned linenum, const char *fmt, va_list args)
{
    if (linenum == 0) {
        _boss9->printf("Error: line %d: ", linenum);
    } else {
        _boss9->printf("Error: ");
    }
    _boss9->vprintf(fmt, args);
    _boss9->printf("\n");
}

void
Emulator::printInstructions(uint16_t addr, uint16_t n)
{
    while (n-- > 0) {
        uint16_t instAddr = addr;
        const Opcode* opcode = &(opcodeTable[load8(addr++)]);
        Op prevOp = Op::NOP;
        Op op = opcode->op;
        
        if (op == Op::Page2 || op == Op::Page3) {
            prevOp = op;
            opcode = &(opcodeTable[load8(addr++)]);
        }
        
        // Do the addr mode
        uint16_t ea = 0;
        int16_t relAddr = 0;
        uint16_t value = 0;
        int16_t offset = 0;
        const char* longBranch = "";
        const char* indexReg = nullptr;
        const char* offsetReg = nullptr;
        bool indirect = false;
        int8_t autoInc = 0;
        Adr addrMode = opcode->adr;
        
        switch(addrMode) {
            case Adr::None:
            case Adr::Inherent:
                break;
            case Adr::Direct:
                ea = load8(addr++);
                break;
            case Adr::Extended:
                ea = load16(addr);
                addr += 2;
                break;
            case Adr::Immed8:
                value = load8(addr++);
                break;
            case Adr::Immed16:
                value = load16(addr);
                addr += 2;
                break;
                
            case Adr::RelL:
                relAddr = int16_t(load16(addr));
                addr += 2;
                longBranch = "l";
                break;
            case Adr::Rel:
                relAddr = int8_t(load8(addr++));
                break;
          case Adr::RelP:
                if (prevOp == Op::Page2) {
                    relAddr = int16_t(load16(addr));
                    addr += 2;
                    longBranch = "l";
                    addrMode = Adr::RelL;
                } else {
                    relAddr = int8_t(load8(addr++));
                    addrMode = Adr::Rel;
                }
                break;
            case Adr::Indexed: {
                uint8_t postbyte = load8(addr++);
                
                // Load value of RR reg in ea
                switch (RR(postbyte & 0b01100000)) {
                    case RR::X: indexReg = "X"; break;
                    case RR::Y: indexReg = "Y"; break;
                    case RR::U: indexReg = "U"; break;
                    case RR::S: indexReg = "S"; break;
                }
                
                if ((postbyte & 0x80) == 0) {
                    // Constant offset direct (5 bit signed)
                    offset = postbyte & 0x1f;
                    if (offset & 0x10) {
                        offset |= 0xe0;
                    }
                } else {
                    switch(IdxMode(postbyte & IdxModeMask)) {
                        case IdxMode::ConstRegNoOff   : offset = 0; break;
                        case IdxMode::ConstReg8Off    : offset = int8_t(load8(addr)); addr += 1; break;
                        case IdxMode::ConstReg16Off   : offset = int16_t(load16(addr)); addr += 2; break;
                        case IdxMode::AccAOffReg      : offsetReg = "A"; break;
                        case IdxMode::AccBOffReg      : offsetReg = "B"; break;
                        case IdxMode::AccDOffReg      : offsetReg = "D"; break;
                        case IdxMode::Inc1Reg         : autoInc = 1; break;
                        case IdxMode::Inc2Reg         : autoInc = 2; break;
                        case IdxMode::Dec1Reg         : autoInc = -1; break;
                        case IdxMode::Dec2Reg         : autoInc = -2; break;
                        case IdxMode::ConstPC8Off     : offset = int8_t(load8(addr)); addr += 1; indexReg = "PC"; break;
                        case IdxMode::ConstPC16Off    : offset = _pc + int16_t(load16(addr)); addr += 2; indexReg = "PC"; break;
                        case IdxMode::Extended:
                            offset = load16(addr);
                            addr += 2;
                            indexReg = nullptr;
                            break;
                    }
                    
                    if (postbyte & IndexedIndMask) {
                        // indirect
                        indirect = true;
                    }
                }
                    
                break;
            }
        }
        
        _boss9->printf("[$%04x]    %s%s%s", instAddr, longBranch, opToString(op), regToString(opcode->reg, prevOp));

        switch(addrMode) {
            case Adr::None:
            case Adr::Inherent: break;
            case Adr::Direct:   _boss9->printf("  <$%02x", ea); break;
            case Adr::Extended: _boss9->printf("  >$%04x", ea); break;
            case Adr::Immed16:  _boss9->printf("  #$%04x", value); break;
            case Adr::Rel:      _boss9->printf("  %d", relAddr); break;
            case Adr::RelL:     _boss9->printf("  %d", relAddr); break;
            case Adr::RelP:     break;
            case Adr::Immed8:
                if (op == Op::TFR || op == Op::EXG) {
                    _boss9->printf("  %s,%s", regToString(Reg(uint8_t(value) >> 4), prevOp),
                                              regToString(Reg(uint8_t(value) & 0xf), prevOp));
                } else if (op == Op::PSH || op == Op::PUL) {
                    _boss9->printf("  ");
                    
                    const char* pushRegs[8] = { "CC", "A", "B", "DP", "X", "Y", "S", "PC" };
                    bool first = true;
                    
                    for (int i = 0; i < 8; ++i) {
                        if ((value & (0x01 << i)) != 0) {
                            const char* r = pushRegs[i];
                            if (r[0] == 'S' && opcode->reg == Reg::S) {
                                r = "U";
                            }
                            if (!first) {
                                _boss9->printf(",");
                            }
                            _boss9->printf("%s", r);
                            first = false;
                        }
                    }
                } else {
                    _boss9->printf("  #$%02x", value);
                }
                break;
            case Adr::Indexed:
                if (indexReg) {
                    if (offsetReg) {
                        if (indirect) {
                            _boss9->printf("  %s,%s", offsetReg, indexReg); break;
                        } else {
                            _boss9->printf("  [%s,%s]", offsetReg, indexReg); break;
                        }
                    } else if (autoInc != 0) {
                        if (indirect) {
                            if (autoInc > 0) {
                                _boss9->printf("  [,%s%s]", (autoInc == 1) ? "+" : "++", indexReg); break;
                            } else {
                                _boss9->printf("  [,%s%s]", indexReg, (autoInc == -1) ? "-" : "--"); break;
                            }
                        } else {
                            if (autoInc > 0) {
                                _boss9->printf("  ,%s%s", (autoInc == 1) ? "+" : "++", indexReg); break;
                            } else {
                                _boss9->printf("  ,%s%s", indexReg, (autoInc == -1) ? "-" : "--"); break;
                            }
                        }
                    } else {
                        if (indirect) {
                            _boss9->printf("  [%d,%s]", offset, indexReg); break;
                        } else {
                            _boss9->printf("  %d,%s", offset, indexReg); break;
                        }
                    }
                } else {
                    // Must be extended indirect
                    _boss9->printf("  [$%04x]", offset, indexReg); break;
                }
                break;

        }
        
        _boss9->printf("\n");
    }
}
    
void Emulator::loadStart()
{
    sRecInfo.init();
    _lineNum = 0;
}

bool Emulator::loadLine(const char* data, bool& finished)
{
    _lineNum++;
    if (!sRecInfo.ParseLine(_lineNum, data)) {
        return false;
    }
    finished = sRecInfo.finished();
    return true;
}

uint16_t Emulator::loadEnd()
{
    sRecInfo.Flush();
    return sRecInfo.startAddr();
}

bool Emulator::execute(RunState runState)
{
    uint32_t instructionsToExecute = InstructionsToExecutePerContinue;
    bool firstTime = true;
    
    while(true) {
        // if runState is not Running we need to ignore a breakpoint at the
        // PC upon entry. Continuing and all the stepping states need to
        // execute the first instruction they encounter
        if (!firstTime || runState == RunState::Running) {
            if (atBreakpoint(_pc)) {
                _boss9->printf("\n*** hit breakpoint at addr $%04x\n\n", _pc);
                _boss9->enterMonitor();
                return true;
            }
        }
        
        firstTime = false;
        
#ifdef TRACE
        _traceBuffer[_traceBufferIndex++] = _pc;
        if (_traceBufferIndex >= TraceBufferSize) {
            _traceBufferIndex = 0;
        }
#endif
        
        uint16_t ea = 0;
        uint8_t opIndex = next8();
        
        const Opcode* opcode = &(opcodeTable[opIndex]);
                
        // Handle address modes
        // If this is an addressing mode that produces a 16 bit effective address
        // it will be placed in ea. If it's immediate or branch relative then the
        // 8 or 16 bit value is placed in _right
        //
        // NOTE: gcc seems to have a problem with emum class and bitfields. It
        // tries to cast the value to an int, which can't be done implicitly with
        // enum class. Moving the value into a bare variable solves the problem.
        //
        Adr adr = opcode->adr;
        
        switch(adr) {
            case Adr::None:
            case Adr::Inherent:
                break;
            case Adr::Direct:
                ea = concat(_dp, next8());
                break;
            case Adr::Extended:
                ea = next16();
                break;
            case Adr::Immed8:
                _right = next8();
                break;
            case Adr::Immed16:
                _right = next16();
                break;
                
            // All the relative addressing modes need to be sign extended to 32 bits
            case Adr::RelL:
                _right = int16_t(next16());
                break;
            case Adr::Rel:
                _right = int8_t(next8());
                break;
          case Adr::RelP:
                if (_prevOp == Op::Page2) {
                    _right = int16_t(next16());
                } else {
                    _right = int8_t(next8());
                }
                break;
            case Adr::Indexed: {
                uint8_t postbyte = next8();
                uint16_t* reg = nullptr;
                
                // Load value of RR reg in ea
                switch (RR(postbyte & 0b01100000)) {
                    case RR::X: reg = &_x; break;
                    case RR::Y: reg = &_y; break;
                    case RR::U: reg = &_u; break;
                    case RR::S: reg = &_s; break;
                }
                
                if ((postbyte & 0x80) == 0) {
                    // Constant offset direct (5 bit signed)
                    int8_t offset = postbyte & 0x1f;
                    if (offset & 0x10) {
                        offset |= 0xe0;
                    }
                    
                    ea = *reg + offset;
                } else {
                    switch(IdxMode(postbyte & IdxModeMask)) {
                        case IdxMode::ConstRegNoOff   : ea = *reg; break;
                        case IdxMode::ConstReg8Off    : ea = *reg + int8_t(load8(_pc)); _pc += 1; break;
                        case IdxMode::ConstReg16Off   : ea = *reg + int16_t(load16(_pc)); _pc += 2; break;
                        case IdxMode::AccAOffReg      : ea = *reg + int8_t(_a); break;
                        case IdxMode::AccBOffReg      : ea = *reg + int8_t(_b); break;
                        case IdxMode::AccDOffReg      : ea = *reg + int16_t(_d); break;
                        case IdxMode::Inc1Reg         : ea = *reg; (*reg) += 1; break;
                        case IdxMode::Inc2Reg         : ea = *reg; (*reg) += 2; break;
                        case IdxMode::Dec1Reg         : (*reg) -= 1; ea = *reg; break;
                        case IdxMode::Dec2Reg         : (*reg) -= 2; ea = *reg; break;
                        case IdxMode::ConstPC8Off     : ea = _pc + int8_t(load8(_pc)); _pc += 1; break;
                        case IdxMode::ConstPC16Off    : ea = _pc + int16_t(load16(_pc)); _pc += 2; break;
                        case IdxMode::Extended        : ea = next16();
                    }
                    
                    if (postbyte & IndexedIndMask) {
                        // indirect from ea
                        ea = load16(ea);
                    }
                }
                    
                break;
            }
        }
        
        // Get left operand
        if (opcode->left == Left::Ld || opcode->left == Left::LdSt) {
            if (opcode->reg == Reg::M8) {
                _left = load8(ea);
            } else if (opcode->reg == Reg::M16) {
                _left = load16(ea);
            } else {
                _left = getReg(opcode);
            }
        }
        
        // Get right operand
        if (opcode->right == Right::Ld8) {
            _right = load8(ea);
        } else if (opcode->right == Right::Ld16) {
            _right = load16(ea);
        }
                
        // Perform operation
        Op op = opcode->op;
        
        switch(op) {
            case Op::ILL:
                _error = Error::Illegal;
                return false;
            case Op::Page2:
            case Op::Page3:
                // We never want to leave execution after a Page2 or Page3.
                // They are basically just part of the next instruction.
                // Since we do the check for whether or not to leave the
                // loop at the end we can ensure that simply by doing
                // a continue to skip that test
                _prevOp = op;
                continue;

            case Op::BHS:
            case Op::BCC: if (!_cc.C) _pc += _right; break;
            case Op::BLO:
            case Op::BCS: if (_cc.C) _pc += _right; break;
            case Op::BEQ: if (_cc.Z) _pc += _right; break;
            case Op::BGE: if (!NxorV()) _pc += _right; break;
            case Op::BGT: if (!(NxorV() || _cc.Z)) _pc += _right; break;
            case Op::BHI: if (!_cc.C && !_cc.Z) _pc += _right; break;
            case Op::BLE: if (NxorV() || _cc.Z) _pc += _right; break;
            case Op::BLS: if (_cc.C || _cc.Z) _pc += _right; break;
            case Op::BLT: if (NxorV()) _pc += _right; break;
            case Op::BMI: if (_cc.N) _pc += _right; break;
            case Op::BNE: if (!_cc.Z) _pc += _right; break;
            case Op::BPL: if (!_cc.N) _pc += _right; break;
            case Op::BRA: _pc += _right; break;
            case Op::BRN: break;
            case Op::BVC: if (!_cc.V) _pc += _right; break;
            case Op::BVS: if (_cc.V) _pc += _right; break;
            case Op::BSR:
                push16(_s, _pc);
                _pc += _right;
                _subroutineDepth += 1;
                break;

            case Op::ABX:
                _x = _x + uint16_t(_b);
                break;
            case Op::ADC:
                _result = _left + _right + (_cc.C ? 1 : 0);
                HNZVC8();
                break;
            case Op::ADD8:
                _result = _left + _right;
                HNZVC8();
                break;
            case Op::ADD16:
                _result = _left + _right;
                xNZVC16();
                break;
            case Op::AND:
                _result = _left & _right;
                xNZ0x8();
                break;
            case Op::ANDCC:
                _ccByte &= _right;
                break;
            case Op::ASL:
                _result = _left << 1;
                xNZxC8();
                _cc.V = (((_left & 0x40) >> 6) ^ ((_left & 0x80) >> 7)) != 0;
                break;
            case Op::ASR:
                _result = int16_t(_left) >> 1;
                if (_left & 0x80) {
                    _result |= 0x80;
                }
                xNZxC8();
                break;
            case Op::BIT:
                _result = _left ^ _right;
                xNZ0x8();
                break;
            case Op::CLR:
                _result = 0;
                _cc.N = false;
                _cc.Z = true;
                _cc.V = false;
                _cc.C = false;
                break;
            case Op::CMP8:
            case Op::SUB8:
                _result = _left - _right;
                xNZVC8();
                break;
            case Op::CMP16:
            case Op::SUB16:
                _result = _left - _right;
                xNZVC16();
                
                // We need to do setReg here because if we're on an altPage these
                // are actually CMP16 and not SUB16
                if (_prevOp != Op::Page2 && _prevOp != Op::Page3 && opcode->op == Op::SUB16) {
                    setReg(opcode->reg, _result);
                }
                break;
            case Op::COM:
                _result = ~_right;
                xNZ018();
                break;
            case Op::CWAI:
                _ccByte ^= _right;
                _cc.E = true;
                push16(_s, _pc);
                push16(_s, _u);
                push16(_s, _y);
                push16(_s, _x);
                push8(_s, _dp);
                push8(_s, _b);
                push8(_s, _a);
                
                // Now what?
                break;
            case Op::DAA: {
                _result = _a;
                uint8_t LSN = _result & 0x0f;
                uint8_t MSN = (_result & 0xf0) >> 4;
                
                // LSN
                if (_cc.H || LSN > 9) {
                    _result += 6;
                }
                
                // MSN
                if (_cc.C || (MSN > 9) || (MSN > 8 && LSN > 9)) {
                    _result += 0x60;
                }
                xNZ0C8();
                _a = _result;
                break;
            }
            case Op::DEC:
                _result = _left - 1;
                xNZxx8();
                _cc.V = _left == 0x80;
                break;
            case Op::EOR:
                _result = _left ^ _right;
                xNZ0x8();
                break;
            case Op::EXG: {
                uint16_t r1 = getReg(Reg(_right & 0xf));
                uint16_t r2 = getReg(Reg(_right >> 4));
                setReg(Reg(_right & 0xf), r2);
                setReg(Reg(_right >> 4), r1);
                break;
            }
            case Op::INC:
                _result = _left + 1;
                xNZxx8();
                _cc.V = _left == 0x7f;
                break;
            case Op::JMP:
            case Op::JSR:
                if (ea >= SystemAddrStart) {
                    // This is possibly a system call
                    if (!_boss9->call(Func(ea))) {
                        return true;
                    }
                } else {
                    if (op == Op::JSR) {
                        push16(_s, _pc);
                    }
                    _pc = ea;
                    if (op == Op::JSR) {
                        _subroutineDepth += 1;
                    }
                }
                break;
            case Op::LD8:
                _result = _right;
                xNZ0x8();
                break;
            case Op::LD16:
                _result = _right;
                xNZ0x16();
                break;
            case Op::LEA:
                _result = ea;
                if (opcode->reg == Reg::X || opcode->reg == Reg::Y) {
                    _cc.Z = _result == 0;
                }
                break;
            case Op::LSR:
                _result = _left >> 1;
                x0ZxC8();
                break;
            case Op::MUL:
                _d = _a * _b;
                _cc.Z = _d == 0;
                _cc.C = _b & 0x80;
                break;
            case Op::NEG:
                _result = -_left;
                xNZxC8();
                _cc.V = _left == 0x80;
                break;
            case Op::NOP:
                break;
            case Op::OR:
                _result = _left | _right;
                xNZ0x8();
                break;
            case Op::ORCC:
                _ccByte |= _right;
                break;
            case Op::PSH:
            case Op::PUL: {
                // bit pattern to push or pull are in _right
                uint16_t& stack = (opcode->reg == Reg::U) ? _u : _s;
                if (opcode->op == Op::PSH) {
                    if (_right & 0x80) push16(stack, _pc);
                    if (_right & 0x40) push16(stack, (opcode->reg == Reg::U) ? _s : _u);
                    if (_right & 0x20) push16(stack, _y);
                    if (_right & 0x10) push16(stack, _x);
                    if (_right & 0x08) push8(stack, _dp);
                    if (_right & 0x04) push8(stack, _b);
                    if (_right & 0x02) push8(stack, _a);
                    if (_right & 0x01) push8(stack, _ccByte);
                } else {
                    if (_right & 0x01) _ccByte = pop8(stack);
                    if (_right & 0x02) _a = pop8(stack);
                    if (_right & 0x04) _b = pop8(stack);
                    if (_right & 0x08) _dp = pop8(stack);
                    if (_right & 0x10) _x = pop16(stack);
                    if (_right & 0x20) _y = pop16(stack);
                    if (_right & 0x40) {
                        if (opcode->reg == Reg::U) {
                            _s = pop16(stack);
                        } else {
                            _u = pop16(stack);
                        }
                    }
                    if (_right & 0x80) _pc = pop16(stack);
                }
                break;
            }
            case Op::ROL:
                _result = _left << 1;
                if (_cc.C) {
                    _result |= 0x01;
                }
                xNZxC8();
                _cc.V = (((_left & 0x40) >> 6) ^ ((_left & 0x80) >> 7)) != 0;
                break;
            case Op::ROR:
                _result = _left >> 1;
                xNZxx8();
                if (_cc.C) {
                    _result |= 0x80;
                }
                if (_left & 0x01) {
                    _cc.C = true;
                }
                break;
            case Op::RTI:
                if (_cc.E) {
                    _a = pop8(_s);
                    _b = pop8(_s);
                    _dp = pop8(_s);
                    _x = pop16(_s);
                    _y = pop16(_s);
                    _u = pop16(_s);
                }
                _pc = pop16(_s);
                break;
            case Op::RTS:
                _pc = pop16(_s);
                _subroutineDepth -= 1;
                if (_lastRunState != RunState::Running && _subroutineDepth == 0) {
                    _boss9->printf("\n*** step %s, stopped at addr $%04x\n\n",
                            (_lastRunState == RunState::StepOver) ? "over" : "out", _pc);
                    _boss9->enterMonitor();
                    return true;
                }
                break;
            case Op::SBC:
                _result = _left - _right - (_cc.C ? 1 : 0);
                xNZVC8();
                break;
            case Op::SEX:
                _a = (_b & 0x80) ? 0xff : 0;
                xNZ0x8();
                break;
            case Op::ST8:
                xNZ0x8();
                break;
            case Op::ST16: // All done in pre and post processing
                xNZ0x16();
                break;
            case Op::SWI:
                _cc.E = true;
                push16(_s, _pc);
                push16(_s, _u);
                push16(_s, _y);
                push16(_s, _x);
                push8(_s, _dp);
                push8(_s, _b);
                push8(_s, _a);
                _cc.I = true;
                _cc.F = true;
                if (_prevOp == Op::Page3) {
                    _pc = load16(0xfff2);
                } else if (_prevOp == Op::Page2) {
                    _pc = load16(0xfff4);
                } else {
                    _pc = load16(0xfffa);
                }
                break;
            case Op::SYNC:
                // Now what?
                break;
            case Op::TFR:
                setReg(Reg(_right & 0xf), getReg(Reg(_right >> 4)));
                break;
            case Op::TST:
                _result = _left - 0;
                xNZ0x8();
                break;
            case Op::FIRQ:
            case Op::IRQ:
            case Op::NMI:
            case Op::RESTART:
                // Now what?
                break;
        }
        
        // Store _result
        if (opcode->right == Right::St8) {
            store8(ea, _left);
        } else if (opcode->right == Right::St16) {
            store16(ea, _left);
        } else if (opcode->left == Left::St || opcode->left == Left::LdSt) {
            if (opcode->right == Right::St8 || opcode->reg == Reg::M8) {
                store8(ea, _result);
            } else if (opcode->right == Right::St16 || opcode->reg == Reg::M16) {
                store16(ea, _result);
            } else {
                setReg(opcode, _result);
            }
        }
        
        _prevOp = opcode->op;
        
        // Step handling
        //
        //  Step In     - Go back to monitor after each instruction executed
        //  Step Over   - Behave like Step In unless current instruction is
        //                BSR or JSR in which case we run until that subroutine
        //                returns.
        //  Step Out    - Run until we return from current subroutine. If none
        //                then we never enter monitor until program exits of ESC.
        //
        // When doing Step Over or Step Out we need to stop when we hit RTS. but
        // there might be nested subroutines so we have to keep track of the
        // depth using _subroutineDepth. When we execute BSR or JSR we increment
        // and when we hit RTS we decrement. When it hits 0 we stop. For Step Over
        // we start with _subroutineDepth = 0. Entering the current BSR or JSR
        // increments to 0 and the matching RTS decrements back to 0 and we
        // enter the monitor. If there are nested subroutines _subroutineDepth
        // keeps track of them so we return on the correct RTS. For Step Out
        // we set _subroutineDepth = 1 so the next RTS we see will stop.
        //
        if (runState != RunState::Running) {
            _lastRunState = runState;
        }
        
        bool handleStepOverLikeStepIn = false;
        
        if (runState == RunState::StepOver) {
            if ((_prevOp != Op::BSR && _prevOp != Op::JSR) ||
                    (_prevOp == Op::JSR && ea >= SystemAddrStart)) {
                handleStepOverLikeStepIn = true;
            } else {
                _subroutineDepth = 1;
            }
        }
        
        if (runState == RunState::StepIn || handleStepOverLikeStepIn) {
            _boss9->printf("\n*** step %s, stopped at addr $%04x\n\n",
                    (_lastRunState == RunState::StepIn) ? "in" : "over", _pc);
            _boss9->enterMonitor();
            return true;
        }
        
        if (runState == RunState::StepOut) {
            _subroutineDepth = 1;
        }

        runState = RunState::Running;
        
        if (--instructionsToExecute == 0) {
            return true;
        }
    }
}

void Emulator::readOnlyAddr(uint16_t addr)
{
    _boss9->printf("Address $%04x is read-only\n", addr);
}

void Emulator::checkActiveBreakpoints()
{
    _haveBreakpoints = false;
    for (auto it : _breakpoints) {
        if (it.status == BPStatus::Enabled) {
            _haveBreakpoints = true;
            return;
        }
    }
}

bool Emulator::breakpoint(uint8_t i, BreakpointEntry& entry) const
{
    if (i >= NumBreakpoints || _breakpoints[i].status == BPStatus::Empty) {
        return false;
    }
    entry = _breakpoints[i];
    return true;
}

bool Emulator::setBreakpoint(uint16_t addr, uint8_t& i)
{
    i = 0;
    for (auto &it : _breakpoints) {
        if (it.status == BPStatus::Empty) {
            it.status = BPStatus::Enabled;
            it.addr = addr;
            _haveBreakpoints = true;
            return true;
        }
        i += 1;
    }
    return false;
}

bool Emulator::clearBreakpoint(uint8_t i)
{
    // Clear the passed breakpoint and move all the others past it up one
    if (i >= NumBreakpoints || _breakpoints[i].status == BPStatus::Empty) {
        return false;
    }
    
    for ( ; i < NumBreakpoints - 1; ++i) {
        _breakpoints[i] = _breakpoints[i + 1];
    }
    _breakpoints[i].status = BPStatus::Empty;
    
    checkActiveBreakpoints();
    return true;
}

bool Emulator::clearAllBreakpoints()
{
    for (auto &it : _breakpoints) {
        it.status = BPStatus::Empty;
    }
    checkActiveBreakpoints();
    return true;
}

bool Emulator::disableBreakpoint(uint8_t i)
{
    if (i >= NumBreakpoints || _breakpoints[i].status == BPStatus::Empty) {
        return false;
    }
    
    _breakpoints[i].status = BPStatus::Disabled;
    checkActiveBreakpoints();
    return true;
}

bool Emulator::disableAllBreakpoints()
{
    for (auto &it : _breakpoints) {
        if (it.status == BPStatus::Enabled) {
            it.status = BPStatus::Disabled;
        }
    }
    checkActiveBreakpoints();
    return true;
}

bool Emulator::enableBreakpoint(uint8_t i)
{
    if (i >= NumBreakpoints || _breakpoints[i].status == BPStatus::Empty) {
        return false;
    }
    
    _breakpoints[i].status = BPStatus::Enabled;
    checkActiveBreakpoints();
    return true;
}

bool Emulator::enableAllBreakpoints()
{
    for (auto &it : _breakpoints) {
        if (it.status == BPStatus::Disabled) {
            it.status = BPStatus::Enabled;
        }
    }
    checkActiveBreakpoints();
    return true;
}

// TODO:
//
// - Handle SYNC and CWAI waiting for interrupt
// - Handle firing of IRQ, FIRQ, NMI and RESTART
// - Handle CC. Have a post op deal with most of it, based on value in Opcode
// - Op handling has zero or more of left, right and ea set in addr handling and reg pre op sections.
//      Do proper setting of these in all cases. Need more than op.reg to do this. Need leftReg, rightReg, etc.
// - Add a Reg.M which uses ea to load data at ea (8 and 16 bit version)
// - Add store Reg value to opcode to store value in post op

// - Did I get ROR and ROL right WRT C flag?
