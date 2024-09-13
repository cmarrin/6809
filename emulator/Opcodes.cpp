/*-------------------------------------------------------------------------
    This source file is a part of the MC6809 Simulator
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2024, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/
//
//  Opcodes.cpp
//  6809 Opcodes
//
//  Created by Chris Marrin on 4/22/24.
//

#include "Opcodes.h"

using namespace mc6809;

static_assert (sizeof(Opcodes::Opcode) == 3, "Opcode is wrong size");

const char*
Opcodes::opToString(Op op) const
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

static constexpr Opcodes::Opcode opcodeTable[ ] = {
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

const Opcode&
Opcodes::opcode(uint8 op) const
{
    return opcodeTable[op];
}

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

const char*
Emulator::regToString(Reg op, Op prevOp)
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
        
        _boss9->printF("[$%04x]    %s%s%s", instAddr, longBranch, opToString(op), regToString(opcode->reg, prevOp));

        switch(addrMode) {
            case Adr::None:
            case Adr::Inherent: break;
            case Adr::Direct:   _boss9->printF("  <$%02x", ea); break;
            case Adr::Extended: _boss9->printF("  >$%04x", ea); break;
            case Adr::Immed16:  _boss9->printF("  #$%04x", value); break;
            case Adr::Rel:      _boss9->printF("  %d", relAddr); break;
            case Adr::RelL:     _boss9->printF("  %d", relAddr); break;
            case Adr::RelP:     break;
            case Adr::Immed8:
                if (op == Op::TFR || op == Op::EXG) {
                    _boss9->printF("  %s,%s", regToString(Reg(uint8_t(value) >> 4), prevOp),
                                              regToString(Reg(uint8_t(value) & 0xf), prevOp));
                } else if (op == Op::PSH || op == Op::PUL) {
                    _boss9->printF("  ");
                    
                    const char* pushRegs[8] = { "CC", "A", "B", "DP", "X", "Y", "S", "PC" };
                    bool first = true;
                    
                    for (int i = 0; i < 8; ++i) {
                        if ((value & (0x01 << i)) != 0) {
                            const char* r = pushRegs[i];
                            if (r[0] == 'S' && opcode->reg == Reg::S) {
                                r = "U";
                            }
                            if (!first) {
                                _boss9->printF(",");
                            }
                            _boss9->printF("%s", r);
                            first = false;
                        }
                    }
                } else {
                    _boss9->printF("  #$%02x", value);
                }
                break;
            case Adr::Indexed:
                if (indexReg) {
                    if (offsetReg) {
                        if (indirect) {
                            _boss9->printF("  %s,%s", offsetReg, indexReg); break;
                        } else {
                            _boss9->printF("  [%s,%s]", offsetReg, indexReg); break;
                        }
                    } else if (autoInc != 0) {
                        if (indirect) {
                            if (autoInc > 0) {
                                _boss9->printF("  [,%s%s]", (autoInc == 1) ? "+" : "++", indexReg); break;
                            } else {
                                _boss9->printF("  [,%s%s]", indexReg, (autoInc == -1) ? "-" : "--"); break;
                            }
                        } else {
                            if (autoInc > 0) {
                                _boss9->printF("  ,%s%s", (autoInc == 1) ? "+" : "++", indexReg); break;
                            } else {
                                _boss9->printF("  ,%s%s", indexReg, (autoInc == -1) ? "-" : "--"); break;
                            }
                        }
                    } else {
                        if (indirect) {
                            _boss9->printF("  [%d,%s]", offset, indexReg); break;
                        } else {
                            _boss9->printF("  %d,%s", offset, indexReg); break;
                        }
                    }
                } else {
                    // Must be extended indirect
                    _boss9->printF("  [$%04x]", offset); break;
                }
                break;

        }
        
        _boss9->printF("\n");
    }
}

