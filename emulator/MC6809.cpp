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

static constexpr Opcode opcodeTable[ ] = {
    /*00*/  	{ Op::NEG	  , Left::LdSt, Right::None , Adr::Direct	, Reg::M8   },
    /*01*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*02*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*03*/  	{ Op::COM	  , Left::LdSt, Right::None , Adr::Direct	, Reg::M8   },
    /*04*/  	{ Op::LSR	  , Left::LdSt, Right::None , Adr::Direct	, Reg::M8   },
    /*05*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*06*/  	{ Op::ROR	  , Left::LdSt, Right::None , Adr::Direct	, Reg::M8   },
    /*07*/  	{ Op::ASR	  , Left::LdSt, Right::None , Adr::Direct	, Reg::M8   },
    /*08*/  	{ Op::ASL	  , Left::LdSt, Right::None , Adr::Direct	, Reg::M8   },
    /*09*/  	{ Op::ROL	  , Left::LdSt, Right::None , Adr::Direct	, Reg::M8   },
    /*0A*/  	{ Op::DEC	  , Left::LdSt, Right::None , Adr::Direct	, Reg::M8   },
    /*0B*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*0C*/  	{ Op::INC	  , Left::LdSt, Right::None , Adr::Direct	, Reg::M8   },
    /*0D*/  	{ Op::TST	  , Left::Ld  , Right::None , Adr::Direct	, Reg::M8   },
    /*0E*/  	{ Op::JMP	  , Left::None, Right::None , Adr::Direct	, Reg::None },
    /*0F*/  	{ Op::CLR	  , Left::St  , Right::None  , Adr::Direct	, Reg::M8   },
    /*10*/  	{ Op::Page2	  , Left::None, Right::None , Adr::None	    , Reg::None },
    /*11*/  	{ Op::Page3	  , Left::None, Right::None , Adr::None	    , Reg::None },
    /*12*/  	{ Op::NOP	  , Left::None, Right::None , Adr::Inherent , Reg::None },
    /*13*/  	{ Op::SYNC	  , Left::None, Right::None , Adr::Inherent	, Reg::None },
    /*14*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*15*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*16*/  	{ Op::BRA	  , Left::None, Right::None , Adr::RelL	    , Reg::None },
    /*17*/  	{ Op::BSR	  , Left::None, Right::None , Adr::RelL	    , Reg::None },
    /*18*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*19*/  	{ Op::DAA	  , Left::None, Right::None , Adr::Inherent	, Reg::None },
    /*1A*/  	{ Op::ORCC	  , Left::None, Right::None , Adr::Immed8	, Reg::None },
    /*1B*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*1C*/  	{ Op::ANDCC	  , Left::None, Right::None , Adr::Immed8	, Reg::None },
    /*1D*/  	{ Op::SEX	  , Left::None, Right::None , Adr::Inherent	, Reg::None },
    /*1E*/  	{ Op::EXG	  , Left::None, Right::None , Adr::Immed8	, Reg::None },
    /*1F*/  	{ Op::TFR	  , Left::None, Right::None , Adr::Immed8	, Reg::None },
    /*20*/  	{ Op::BRA	  , Left::None, Right::None , Adr::Rel	    , Reg::None },
    /*21*/  	{ Op::BRN	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*22*/  	{ Op::BHI	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*23*/  	{ Op::BLS	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*24*/  	{ Op::BHS	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*25*/  	{ Op::BLO	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*26*/  	{ Op::BNE	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*27*/  	{ Op::BEQ	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*28*/  	{ Op::BVC	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*29*/  	{ Op::BVS	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*2A*/  	{ Op::BPL	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*2B*/  	{ Op::BMI	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*2C*/  	{ Op::BGE	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*2D*/  	{ Op::BLT	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*2E*/  	{ Op::BGT	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*2F*/  	{ Op::BLE	  , Left::None, Right::None , Adr::RelP	    , Reg::None },
    /*30*/  	{ Op::LEA	  , Left::St  , Right::None , Adr::Indexed	, Reg::X    },
    /*31*/  	{ Op::LEA	  , Left::None, Right::None , Adr::Indexed	, Reg::Y    },
    /*32*/  	{ Op::LEA	  , Left::None, Right::None , Adr::Indexed	, Reg::S    },
    /*33*/  	{ Op::LEA	  , Left::None, Right::None , Adr::Indexed	, Reg::U    },
    /*34*/  	{ Op::PSH	  , Left::None, Right::None , Adr::Immed8	, Reg::S    },
    /*35*/  	{ Op::PUL	  , Left::None, Right::None , Adr::Immed8	, Reg::S    },
    /*36*/  	{ Op::PSH	  , Left::None, Right::None , Adr::Immed8	, Reg::U    },
    /*37*/  	{ Op::PUL	  , Left::None, Right::None , Adr::Immed8	, Reg::U    },
    /*38*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None	    , Reg::None },
    /*39*/  	{ Op::RTS	  , Left::None, Right::None , Adr::Inherent	, Reg::None },
    /*3A*/  	{ Op::ABX	  , Left::None, Right::None , Adr::Inherent	, Reg::None },
    /*3B*/  	{ Op::RTI	  , Left::None, Right::None , Adr::Inherent	, Reg::None },          // 6 if FIRQ, 15 if IRQ
    /*3C*/  	{ Op::CWAI	  , Left::None, Right::None , Adr::Inherent	, Reg::None },
    /*3D*/  	{ Op::MUL	  , Left::None, Right::None , Adr::Inherent	, Reg::None },
    /*3E*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None	    , Reg::None },
    /*3F*/  	{ Op::SWI	  , Left::None, Right::None , Adr::Inherent	, Reg::None },
    /*40*/  	{ Op::NEG	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::A    },
    /*41*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None	    , Reg::None },
    /*42*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*43*/  	{ Op::COM	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::A    },
    /*44*/  	{ Op::LSR	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::A    },
    /*45*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*46*/  	{ Op::ROR	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::A    },
    /*47*/  	{ Op::ASR	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::A    },
    /*48*/  	{ Op::ASL	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::A    },
    /*49*/  	{ Op::ROL	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::A    },
    /*4A*/  	{ Op::DEC	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::A    },
    /*4B*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*4C*/  	{ Op::INC	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::A    },
    /*4D*/  	{ Op::TST	  , Left::Ld  , Right::None , Adr::Inherent	, Reg::A    },
    /*4E*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None	    , Reg::None },
    /*4F*/  	{ Op::CLR	  , Left::St  , Right::None , Adr::Inherent	, Reg::A    },
    /*50*/  	{ Op::NEG	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::B    },
    /*51*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None	    , Reg::None },
    /*52*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*53*/  	{ Op::COM	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::B    },
    /*54*/  	{ Op::LSR	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::B    },
    /*55*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*56*/  	{ Op::ROR	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::B    },
    /*57*/  	{ Op::ASR	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::B    },
    /*58*/  	{ Op::ASL	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::B    },
    /*59*/  	{ Op::ROL	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::B    },
    /*5A*/  	{ Op::DEC	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::B    },
    /*5B*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*5C*/  	{ Op::INC	  , Left::LdSt, Right::None , Adr::Inherent	, Reg::B    },
    /*5D*/  	{ Op::TST	  , Left::Ld  , Right::None , Adr::Inherent	, Reg::B    },
    /*5E*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None	    , Reg::None },
    /*5F*/  	{ Op::CLR	  , Left::St  , Right::None , Adr::Inherent	, Reg::B    },
    /*60*/  	{ Op::NEG	  , Left::LdSt, Right::None , Adr::Indexed	, Reg::M8   },
    /*61*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None	    , Reg::None },
    /*62*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*63*/  	{ Op::COM	  , Left::LdSt, Right::None , Adr::Indexed	, Reg::M8   },
    /*64*/  	{ Op::LSR	  , Left::LdSt, Right::None , Adr::Indexed	, Reg::M8   },
    /*65*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*66*/  	{ Op::ROR	  , Left::LdSt, Right::None , Adr::Indexed	, Reg::M8   },
    /*67*/  	{ Op::ASR	  , Left::LdSt, Right::None , Adr::Indexed	, Reg::M8   },
    /*68*/  	{ Op::ASL	  , Left::LdSt, Right::None , Adr::Indexed	, Reg::M8   },
    /*69*/  	{ Op::ROL	  , Left::LdSt, Right::None , Adr::Indexed	, Reg::M8   },
    /*6A*/  	{ Op::DEC	  , Left::LdSt, Right::None , Adr::Indexed	, Reg::M8   },
    /*6B*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*6C*/  	{ Op::INC	  , Left::LdSt, Right::None , Adr::Indexed	, Reg::M8   },
    /*6D*/  	{ Op::TST	  , Left::Ld  , Right::None , Adr::Indexed	, Reg::M8   },
    /*6E*/  	{ Op::JMP	  , Left::None, Right::None , Adr::Indexed	, Reg::None },
    /*6F*/  	{ Op::CLR	  , Left::St  , Right::None , Adr::Indexed	, Reg::M8   },
    /*70*/  	{ Op::NEG	  , Left::LdSt, Right::None , Adr::Extended	, Reg::M8   },
    /*71*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None	    , Reg::None },
    /*72*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*73*/  	{ Op::COM	  , Left::LdSt, Right::None , Adr::Extended	, Reg::M8   },
    /*74*/  	{ Op::LSR	  , Left::LdSt, Right::None , Adr::Extended	, Reg::M8   },
    /*75*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*76*/  	{ Op::ROR	  , Left::LdSt, Right::None , Adr::Extended	, Reg::M8   },
    /*77*/  	{ Op::ASR	  , Left::LdSt, Right::None , Adr::Extended	, Reg::M8   },
    /*78*/  	{ Op::ASL	  , Left::LdSt, Right::None , Adr::Extended	, Reg::M8   },
    /*79*/  	{ Op::ROL	  , Left::LdSt, Right::None , Adr::Extended	, Reg::M8   },
    /*7A*/  	{ Op::DEC	  , Left::LdSt, Right::None , Adr::Extended	, Reg::M8   },
    /*7B*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*7C*/  	{ Op::INC	  , Left::LdSt, Right::None , Adr::Extended	, Reg::M8   },
    /*7D*/  	{ Op::TST	  , Left::Ld  , Right::None , Adr::Extended	, Reg::M8   },
    /*7E*/  	{ Op::JMP	  , Left::None, Right::None , Adr::Extended	, Reg::None },
    /*7F*/  	{ Op::CLR	  , Left::St  , Right::None , Adr::Extended	, Reg::M8   },
    /*80*/  	{ Op::SUB8	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::A    },
    /*81*/  	{ Op::CMP8	  , Left::Ld  , Right::None , Adr::Immed8	, Reg::A    },
    /*82*/  	{ Op::SBC	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::A    },
    /*83*/  	{ Op::SUB16	  , Left::Ld  , Right::None , Adr::Immed16	, Reg::D    },
    /*84*/  	{ Op::AND	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::A    },
    /*85*/  	{ Op::BIT	  , Left::Ld  , Right::None , Adr::Immed8	, Reg::A    },
    /*86*/  	{ Op::LD8	  , Left::St  , Right::None , Adr::Immed8	, Reg::A    },
    /*87*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*88*/  	{ Op::EOR	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::A    },
    /*89*/  	{ Op::ADC	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::A    },
    /*8A*/  	{ Op::OR	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::A    },
    /*8B*/  	{ Op::ADD8	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::A    },
    /*8C*/  	{ Op::CMP16	  , Left::Ld  , Right::None , Adr::Immed16	, Reg::X    },
    /*8D*/  	{ Op::BSR	  , Left::None, Right::None , Adr::Rel      , Reg::None },
    /*8E*/  	{ Op::LD16	  , Left::St  , Right::None , Adr::Immed16	, Reg::X    },
    /*8F*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*90*/  	{ Op::SUB8	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::A    },
    /*91*/  	{ Op::CMP8	  , Left::Ld  , Right::Ld8  , Adr::Direct	, Reg::A    },
    /*92*/  	{ Op::SBC	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::A    },
    /*93*/  	{ Op::SUB16	  , Left::Ld  , Right::Ld16 , Adr::Direct	, Reg::D    },
    /*94*/  	{ Op::AND	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::A    },
    /*95*/  	{ Op::BIT	  , Left::Ld  , Right::Ld8  , Adr::Direct	, Reg::A    },
    /*96*/  	{ Op::LD8	  , Left::St  , Right::Ld8  , Adr::Direct	, Reg::A    },
    /*97*/  	{ Op::ST8	  , Left::Ld  , Right::St8  , Adr::Direct	, Reg::A    },
    /*98*/  	{ Op::EOR	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::A    },
    /*99*/  	{ Op::ADC	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::A    },
    /*9A*/  	{ Op::OR	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::A    },
    /*9B*/  	{ Op::ADD8	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::A    },
    /*9C*/  	{ Op::CMP16	  , Left::Ld  , Right::Ld8  , Adr::Direct	, Reg::X    },
    /*9D*/  	{ Op::JSR	  , Left::None, Right::None , Adr::Direct	, Reg::None },
    /*9E*/  	{ Op::LD16	  , Left::St  , Right::Ld16 , Adr::Direct	, Reg::X    },
    /*9F*/  	{ Op::ST16	  , Left::Ld  , Right::St16 , Adr::Direct	, Reg::X    },
    /*A0*/  	{ Op::SUB8	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::A    },
    /*A1*/  	{ Op::CMP8	  , Left::Ld  , Right::Ld8  , Adr::Indexed	, Reg::A    },
    /*A2*/  	{ Op::SBC	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::A    },
    /*A3*/  	{ Op::SUB16	  , Left::Ld  , Right::Ld16 , Adr::Indexed	, Reg::D    },
    /*A4*/  	{ Op::AND	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::A    },
    /*A5*/  	{ Op::BIT	  , Left::Ld  , Right::Ld8  , Adr::Indexed	, Reg::A    },
    /*A6*/  	{ Op::LD8	  , Left::St  , Right::Ld8  , Adr::Indexed	, Reg::A    },
    /*A7*/  	{ Op::ST8	  , Left::Ld  , Right::St8  , Adr::Indexed	, Reg::A    },
    /*A8*/  	{ Op::EOR	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::A    },
    /*A9*/  	{ Op::ADC	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::A    },
    /*AA*/  	{ Op::OR	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::A    },
    /*AB*/  	{ Op::ADD8	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::A    },
    /*AC*/  	{ Op::CMP16	  , Left::Ld  , Right::Ld16 , Adr::Indexed	, Reg::X    },
    /*AD*/  	{ Op::JSR	  , Left::None, Right::None , Adr::Indexed	, Reg::None },
    /*AE*/  	{ Op::LD16	  , Left::St  , Right::Ld16 , Adr::Indexed	, Reg::X    },
    /*AF*/  	{ Op::ST16	  , Left::Ld  , Right::St16 , Adr::Indexed	, Reg::X    },
    /*B0*/  	{ Op::SUB8	  , Left::LdSt, Right::Ld8  , Adr::Extended	, Reg::A    },
    /*B1*/  	{ Op::CMP8	  , Left::Ld  , Right::Ld8  , Adr::Extended	, Reg::A    },
    /*B2*/  	{ Op::SBC	  , Left::LdSt, Right::Ld8  , Adr::Extended	, Reg::A    },
    /*B3*/  	{ Op::SUB16	  , Left::Ld  , Right::Ld16 , Adr::Extended	, Reg::D    },
    /*B4*/  	{ Op::AND	  , Left::LdSt, Right::Ld8  , Adr::Extended	, Reg::A    },
    /*B5*/  	{ Op::BIT	  , Left::Ld  , Right::Ld8  , Adr::Extended	, Reg::A    },
    /*B6*/  	{ Op::LD8	  , Left::St  , Right::Ld8  , Adr::Extended	, Reg::A    },
    /*B7*/  	{ Op::ST8	  , Left::Ld  , Right::St8  , Adr::Extended	, Reg::A    },
    /*B8*/  	{ Op::EOR	  , Left::LdSt, Right::Ld8  , Adr::Extended	, Reg::A    },
    /*B9*/  	{ Op::ADC	  , Left::LdSt, Right::Ld8  , Adr::Extended	, Reg::A    },
    /*BA*/  	{ Op::OR	  , Left::LdSt, Right::Ld8  , Adr::Extended	, Reg::A    },
    /*BB*/  	{ Op::ADD8	  , Left::LdSt, Right::Ld8  , Adr::Extended	, Reg::A    },
    /*BC*/  	{ Op::CMP16	  , Left::Ld  , Right::Ld16 , Adr::Extended	, Reg::X    },
    /*BD*/  	{ Op::JSR	  , Left::None, Right::None , Adr::Extended	, Reg::None },
    /*BE*/  	{ Op::LD16	  , Left::St  , Right::Ld16 , Adr::Extended	, Reg::X    },
    /*BF*/  	{ Op::ST16	  , Left::Ld  , Right::St16 , Adr::Extended	, Reg::X    },
    /*C0*/  	{ Op::SUB8	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::B    },
    /*C1*/  	{ Op::CMP8	  , Left::Ld  , Right::None , Adr::Immed8	, Reg::B    },
    /*C2*/  	{ Op::SBC	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::B    },
    /*C3*/  	{ Op::ADD16	  , Left::LdSt, Right::None , Adr::Immed16	, Reg::D    },
    /*C4*/  	{ Op::AND	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::B    },
    /*C5*/  	{ Op::BIT	  , Left::Ld  , Right::None , Adr::Immed8	, Reg::B    },
    /*C6*/  	{ Op::LD8	  , Left::St  , Right::None , Adr::Immed8	, Reg::B    },
    /*C7*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*C8*/  	{ Op::EOR	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::B    },
    /*C9*/  	{ Op::ADC	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::B    },
    /*CA*/  	{ Op::OR	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::B    },
    /*CB*/  	{ Op::ADD8	  , Left::LdSt, Right::None , Adr::Immed8	, Reg::B    },
    /*CC*/  	{ Op::LD16	  , Left::St  , Right::None , Adr::Immed16	, Reg::D    },
    /*CD*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*CE*/  	{ Op::LD16	  , Left::St  , Right::None , Adr::Immed16	, Reg::U    },
    /*CF*/  	{ Op::ILL	  , Left::None, Right::None , Adr::None     , Reg::None },
    /*D0*/  	{ Op::SUB8	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::B    },
    /*D1*/  	{ Op::CMP8	  , Left::Ld  , Right::Ld8  , Adr::Direct	, Reg::B    },
    /*D2*/  	{ Op::SBC	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::B    },
    /*D3*/  	{ Op::ADD16	  , Left::LdSt, Right::Ld16 , Adr::Direct	, Reg::D    },
    /*D4*/  	{ Op::AND	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::B    },
    /*D5*/  	{ Op::BIT	  , Left::Ld  , Right::Ld8  , Adr::Direct	, Reg::B    },
    /*D6*/  	{ Op::LD8	  , Left::St  , Right::Ld8  , Adr::Direct	, Reg::B    },
    /*D7*/  	{ Op::ST8	  , Left::Ld  , Right::St8  , Adr::Direct	, Reg::B    },
    /*D8*/  	{ Op::EOR	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::B    },
    /*D9*/  	{ Op::ADC	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::B    },
    /*DA*/  	{ Op::OR	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::B    },
    /*DB*/  	{ Op::ADD8	  , Left::LdSt, Right::Ld8  , Adr::Direct	, Reg::B    },
    /*DC*/  	{ Op::LD16	  , Left::St  , Right::Ld16 , Adr::Direct	, Reg::D    },
    /*DD*/  	{ Op::ST16	  , Left::Ld  , Right::St16 , Adr::Direct	, Reg::D    },
    /*DE*/  	{ Op::LD16	  , Left::St  , Right::Ld16 , Adr::Direct	, Reg::U    },
    /*DF*/  	{ Op::ST16	  , Left::Ld  , Right::St16 , Adr::Direct	, Reg::U    },
    /*E0*/  	{ Op::SUB8	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::B    },
    /*E1*/  	{ Op::CMP8	  , Left::Ld  , Right::Ld8  , Adr::Indexed	, Reg::B    },
    /*E2*/  	{ Op::SBC	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::B    },
    /*E3*/  	{ Op::ADD16	  , Left::LdSt, Right::Ld16 , Adr::Indexed	, Reg::D    },
    /*E4*/  	{ Op::AND	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::B    },
    /*E5*/  	{ Op::BIT	  , Left::Ld  , Right::Ld8  , Adr::Indexed	, Reg::B    },
    /*E6*/  	{ Op::LD8	  , Left::St  , Right::Ld8  , Adr::Indexed	, Reg::B    },
    /*E7*/  	{ Op::ST8	  , Left::Ld  , Right::St8  , Adr::Indexed	, Reg::B    },
    /*E8*/  	{ Op::EOR	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::B    },
    /*E9*/  	{ Op::ADC	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::B    },
    /*EA*/  	{ Op::OR	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::B    },
    /*EB*/  	{ Op::ADD8	  , Left::LdSt, Right::Ld8  , Adr::Indexed	, Reg::B    },
    /*EC*/  	{ Op::LD16	  , Left::St  , Right::Ld16 , Adr::Indexed	, Reg::D    },
    /*ED*/  	{ Op::ST16	  , Left::Ld  , Right::St16 , Adr::Indexed	, Reg::D    },
    /*EE*/  	{ Op::LD16	  , Left::St  , Right::Ld16 , Adr::Indexed	, Reg::U    },
    /*EF*/  	{ Op::ST16	  , Left::Ld  , Right::St16 , Adr::Indexed	, Reg::U    },
    /*F0*/  	{ Op::SUB8	  , Left::LdSt, Right::Ld8  , Adr::Extended , Reg::B    },
    /*F1*/  	{ Op::CMP8	  , Left::Ld  , Right::Ld8  , Adr::Extended , Reg::B    },
    /*F2*/  	{ Op::SBC	  , Left::LdSt, Right::Ld8  , Adr::Extended , Reg::B    },
    /*F3*/  	{ Op::ADD16	  , Left::LdSt, Right::Ld16 , Adr::Extended , Reg::D    },
    /*F4*/  	{ Op::AND	  , Left::LdSt, Right::Ld8  , Adr::Extended , Reg::B    },
    /*F5*/  	{ Op::BIT	  , Left::Ld  , Right::Ld8  , Adr::Extended , Reg::B    },
    /*F6*/  	{ Op::LD8	  , Left::St  , Right::Ld8  , Adr::Extended , Reg::B    },
    /*F7*/  	{ Op::ST8	  , Left::Ld  , Right::St8  , Adr::Extended , Reg::B    },
    /*F8*/  	{ Op::EOR	  , Left::LdSt, Right::Ld8  , Adr::Extended , Reg::B    },
    /*F9*/  	{ Op::ADC	  , Left::LdSt, Right::Ld8  , Adr::Extended , Reg::B    },
    /*FA*/  	{ Op::OR	  , Left::LdSt, Right::Ld8  , Adr::Extended , Reg::B    },
    /*FB*/  	{ Op::ADD8	  , Left::LdSt, Right::Ld8  , Adr::Extended , Reg::B    },
    /*FC*/  	{ Op::LD16	  , Left::St  , Right::Ld16 , Adr::Extended , Reg::D    },
    /*FD*/  	{ Op::ST16	  , Left::Ld  , Right::St16 , Adr::Extended , Reg::D    },
    /*FE*/  	{ Op::LD16	  , Left::St  , Right::Ld16 , Adr::Extended , Reg::U    },
    /*FF*/  	{ Op::ST16	  , Left::Ld  , Right::St16 , Adr::Extended , Reg::U    },
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
        _boss9->printf("[%04x]    %s\n", instAddr, opToString(opcode->op));
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

uint16_t Emulator::loadFinish()
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
                return false;
            case Op::Page2:
            case Op::Page3:
                // We never want to leave execution after a Page2 or Page3.
                // They are basically just part of the next instruction.
                // Since we do the check for whether or not to leave the
                // loop at the end we can ensure that simply by doing
                // a continue to skip that test
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
                _pc = ea;
                break;
            case Op::JSR:
                if (ea >= SystemAddrStart) {
                    // This is possibly a system call
                    if (!_boss9->call(this, ea)) {
                        return true;
                    }
                } else {
                    push16(_s, _pc);
                    _pc = ea;
                    _subroutineDepth += 1;
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
