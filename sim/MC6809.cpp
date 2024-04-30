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

using namespace mc6809;

static_assert (sizeof(Opcode) == 4, "Opcode is wrong size");

static constexpr Opcode opcodeTable[ ] = {
    /*00*/  	{ Op::NEG	  , true    , false , Adr::Direct	, Reg::M8   , CCOp::None, 6  },
    /*01*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*02*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*03*/  	{ Op::COM	  , true    , false , Adr::Direct	, Reg::M8   , CCOp::None, 6	 },
    /*04*/  	{ Op::LSR	  , true    , false , Adr::Direct	, Reg::M8   , CCOp::None, 6	 },
    /*05*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*06*/  	{ Op::ROR	  , true    , false , Adr::Direct	, Reg::M8   , CCOp::None, 6	 },
    /*07*/  	{ Op::ASR	  , true    , false , Adr::Direct	, Reg::M8   , CCOp::None, 6	 },
    /*08*/  	{ Op::ASL	  , true    , false , Adr::Direct	, Reg::M8   , CCOp::None, 6	 },
    /*09*/  	{ Op::ROL	  , true    , false , Adr::Direct	, Reg::M8   , CCOp::None, 6	 },
    /*0A*/  	{ Op::DEC	  , true    , false , Adr::Direct	, Reg::M8   , CCOp::None, 6	 },
    /*0B*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*0C*/  	{ Op::INC	  , true    , false , Adr::Direct	, Reg::M8   , CCOp::None, 6	 },
    /*0D*/  	{ Op::TST	  , true    , false , Adr::Direct	, Reg::M8   , CCOp::None, 6	 },
    /*0E*/  	{ Op::JMP	  , false   , false , Adr::Direct	, Reg::None , CCOp::None, 3	 },
    /*0F*/  	{ Op::CLR	  , true    , false , Adr::Direct	, Reg::M8   , CCOp::None, 6	 },
    /*10*/  	{ Op::Page2	  , false   , false , Adr::None	    , Reg::None , CCOp::None, 0	 },
    /*11*/  	{ Op::Page3	  , false   , false , Adr::None	    , Reg::None , CCOp::None, 0	 },
    /*12*/  	{ Op::NOP	  , false   , false , Adr::Inherent , Reg::None , CCOp::None, 2	 },
    /*13*/  	{ Op::SYNC	  , false   , false , Adr::Inherent	, Reg::None , CCOp::None, 4	 },
    /*14*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*15*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*16*/  	{ Op::BRA	  , false   , false , Adr::RelL	    , Reg::None , CCOp::None, 5	 },
    /*17*/  	{ Op::BSR	  , false   , false , Adr::RelL	    , Reg::None , CCOp::None, 9	 },
    /*18*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*19*/  	{ Op::DAA	  , false   , false , Adr::Inherent	, Reg::None , CCOp::None, 2	 },
    /*1A*/  	{ Op::ORCC	  , false   , false , Adr::Immed8	, Reg::None , CCOp::None, 3	 },
    /*1B*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*1C*/  	{ Op::ANDCC	  , false   , false , Adr::Immed8	, Reg::None , CCOp::None, 3	 },
    /*1D*/  	{ Op::SEX	  , false   , false , Adr::Inherent	, Reg::None , CCOp::None, 2	 },
    /*1E*/  	{ Op::EXG	  , false   , false , Adr::Immed8	, Reg::None , CCOp::None, 8	 },
    /*1F*/  	{ Op::TFR	  , false   , false , Adr::Immed8	, Reg::None , CCOp::None, 6	 },
    /*20*/  	{ Op::BRA	  , false   , false , Adr::Rel	    , Reg::None , CCOp::None, 3	 },
    /*21*/  	{ Op::BRN	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*22*/  	{ Op::BHI	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*23*/  	{ Op::BLS	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*24*/  	{ Op::BHS	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*25*/  	{ Op::BLO	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*26*/  	{ Op::BNE	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*27*/  	{ Op::BEQ	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*28*/  	{ Op::BVC	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*29*/  	{ Op::BVS	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*2A*/  	{ Op::BPL	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*2B*/  	{ Op::BMI	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*2C*/  	{ Op::BGE	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*2D*/  	{ Op::BLT	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*2E*/  	{ Op::BGT	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    /*2F*/  	{ Op::BLE	  , false   , false , Adr::RelP	    , Reg::None , CCOp::None, 3	 },
    
    
    /*30*/  	{ Op::LEA	  , false   , false , Adr::Indexed	, Reg::X    , CCOp::None, 4	 },
    /*31*/  	{ Op::LEA	  , false   , false , Adr::Indexed	, Reg::Y    , CCOp::None, 4	 },
    /*32*/  	{ Op::LEA	  , false   , false , Adr::Indexed	, Reg::S    , CCOp::None, 4	 },
    /*33*/  	{ Op::LEA	  , false   , false , Adr::Indexed	, Reg::U    , CCOp::None, 4	 },
    /*34*/  	{ Op::PSH	  , false   , false , Adr::Immed8	, Reg::S    , CCOp::None, 5	 },
    /*35*/  	{ Op::PUL	  , false   , false , Adr::Immed8	, Reg::S    , CCOp::None, 5	 },
    /*36*/  	{ Op::PSH	  , false   , false , Adr::Immed8	, Reg::U    , CCOp::None, 5	 },
    /*37*/  	{ Op::PUL	  , false   , false , Adr::Immed8	, Reg::U    , CCOp::None, 5	 },
    /*38*/  	{ Op::ILL	  , false   , false , Adr::None	    , Reg::None , CCOp::None, 0	 },
    /*39*/  	{ Op::RTS	  , false   , false , Adr::Inherent	, Reg::None , CCOp::None, 5	 },
    /*3A*/  	{ Op::ABX	  , false   , false , Adr::Inherent	, Reg::None , CCOp::None, 3	 },
    /*3B*/  	{ Op::RTI	  , false   , false , Adr::Inherent	, Reg::None , CCOp::None, 6	 },          // 6 if FIRQ, 15 if IRQ
    /*3C*/  	{ Op::CWAI	  , false   , false , Adr::Inherent	, Reg::None , CCOp::None, 20 },
    /*3D*/  	{ Op::MUL	  , false   , false , Adr::Inherent	, Reg::None , CCOp::None, 11 },
    /*3E*/  	{ Op::ILL	  , false   , false , Adr::None	    , Reg::None , CCOp::None, 0	 },
    /*3F*/  	{ Op::SWI	  , false   , false , Adr::Inherent	, Reg::None , CCOp::None, 19 },
    /*40*/  	{ Op::NEG	  , false   , false , Adr::Inherent	, Reg::A    , CCOp::None, 2	 },
    /*41*/  	{ Op::ILL	  , false   , false , Adr::None	    , Reg::None , CCOp::None, 0	 },
    /*42*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*43*/  	{ Op::COM	  , false   , false , Adr::Inherent	, Reg::A    , CCOp::None, 2	 },
    /*44*/  	{ Op::LSR	  , false   , false , Adr::Inherent	, Reg::A    , CCOp::None, 2	 },
    /*45*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*46*/  	{ Op::ROR	  , false   , false , Adr::Inherent	, Reg::A    , CCOp::None, 2	 },
    /*47*/  	{ Op::ASR	  , false   , false , Adr::Inherent	, Reg::A    , CCOp::None, 2	 },
    /*48*/  	{ Op::ASL	  , false   , false , Adr::Inherent	, Reg::A    , CCOp::None, 2	 },
    /*49*/  	{ Op::ROL	  , false   , false , Adr::Inherent	, Reg::A    , CCOp::None, 2	 },
    /*4A*/  	{ Op::DEC	  , false   , false , Adr::Inherent	, Reg::A    , CCOp::None, 2	 },
    /*4B*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*4C*/  	{ Op::INC	  , false   , false , Adr::Inherent	, Reg::A    , CCOp::None, 2	 },
    /*4D*/  	{ Op::TST	  , false   , false , Adr::Inherent	, Reg::A    , CCOp::None, 2	 },
    /*4E*/  	{ Op::ILL	  , false   , false , Adr::None	    , Reg::None , CCOp::None, 0	 },
    /*4F*/  	{ Op::CLR	  , false   , false , Adr::Inherent	, Reg::A    , CCOp::None, 2	 },
    /*50*/  	{ Op::NEG	  , false   , false , Adr::Inherent	, Reg::B    , CCOp::None, 2	 },
    /*51*/  	{ Op::ILL	  , false   , false , Adr::None	    , Reg::None , CCOp::None, 0	 },
    /*52*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*53*/  	{ Op::COM	  , false   , false , Adr::Inherent	, Reg::B    , CCOp::None, 2	 },
    /*54*/  	{ Op::LSR	  , false   , false , Adr::Inherent	, Reg::B    , CCOp::None, 2	 },
    /*55*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*56*/  	{ Op::ROR	  , false   , false , Adr::Inherent	, Reg::B    , CCOp::None, 2	 },
    /*57*/  	{ Op::ASR	  , false   , false , Adr::Inherent	, Reg::B    , CCOp::None, 2	 },
    /*58*/  	{ Op::ASL	  , false   , false , Adr::Inherent	, Reg::B    , CCOp::None, 2	 },
    /*59*/  	{ Op::ROL	  , false   , false , Adr::Inherent	, Reg::B    , CCOp::None, 2	 },
    /*5A*/  	{ Op::DEC	  , false   , false , Adr::Inherent	, Reg::B    , CCOp::None, 2	 },
    /*5B*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*5C*/  	{ Op::INC	  , false   , false , Adr::Inherent	, Reg::B    , CCOp::None, 2	 },
    /*5D*/  	{ Op::TST	  , false   , false , Adr::Inherent	, Reg::B    , CCOp::None, 2	 },
    /*5E*/  	{ Op::ILL	  , false   , false , Adr::None	    , Reg::None , CCOp::None, 0	 },
    /*5F*/  	{ Op::CLR	  , false   , false , Adr::Inherent	, Reg::B    , CCOp::None, 2	 },
    /*60*/  	{ Op::NEG	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 6	 },
    /*61*/  	{ Op::ILL	  , false   , false , Adr::None	    , Reg::None , CCOp::None, 0	 },
    /*62*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*63*/  	{ Op::COM	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 6	 },
    /*64*/  	{ Op::LSR	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 6	 },
    /*65*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*66*/  	{ Op::ROR	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 6	 },
    /*67*/  	{ Op::ASR	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 6	 },
    /*68*/  	{ Op::ASL	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 6	 },
    /*69*/  	{ Op::ROL	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 6	 },
    /*6A*/  	{ Op::DEC	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 6	 },
    /*6B*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*6C*/  	{ Op::INC	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 6	 },
    /*6D*/  	{ Op::TST	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 6	 },
    /*6E*/  	{ Op::JMP	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 3	 },
    /*6F*/  	{ Op::CLR	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 6	 },
    /*70*/  	{ Op::NEG	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 7	 },
    /*71*/  	{ Op::ILL	  , false   , false , Adr::None	    , Reg::None , CCOp::None, 0	 },
    /*72*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*73*/  	{ Op::COM	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 7	 },
    /*74*/  	{ Op::LSR	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 7	 },
    /*75*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*76*/  	{ Op::ROR	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 7	 },
    /*77*/  	{ Op::ASR	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 7	 },
    /*78*/  	{ Op::ASL	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 7	 },
    /*79*/  	{ Op::ROL	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 7	 },
    /*7A*/  	{ Op::DEC	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 7	 },
    /*7B*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*7C*/  	{ Op::INC	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 7	 },
    /*7D*/  	{ Op::TST	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 7	 },
    /*7E*/  	{ Op::JMP	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 4	 },
    /*7F*/  	{ Op::CLR	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 7	 },
    /*80*/  	{ Op::SUB8	  , false   , false , Adr::Immed8	, Reg::A    , CCOp::None, 2	 },
    /*81*/  	{ Op::CMP8	  , false   , false , Adr::Immed8	, Reg::A    , CCOp::None, 2	 },
    /*82*/  	{ Op::SBC	  , false   , false , Adr::Immed8	, Reg::A    , CCOp::None, 2	 },
    /*83*/  	{ Op::SUB16	  , false   , false , Adr::Immed16	, Reg::D    , CCOp::None, 4	 },
    /*84*/  	{ Op::AND	  , false   , false , Adr::Immed8	, Reg::A    , CCOp::None, 2	 },
    /*85*/  	{ Op::BIT	  , false   , false , Adr::Immed8	, Reg::A    , CCOp::None, 2	 },
    /*86*/  	{ Op::LD8	  , false   , false , Adr::Immed8	, Reg::A    , CCOp::None, 2	 },
    /*87*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*88*/  	{ Op::EOR	  , false   , false , Adr::Immed8	, Reg::A    , CCOp::None, 2	 },
    /*89*/  	{ Op::ADC	  , false   , false , Adr::Immed8	, Reg::A    , CCOp::None, 2	 },
    /*8A*/  	{ Op::OR	  , false   , false , Adr::Immed8	, Reg::A    , CCOp::None, 2	 },
    /*8B*/  	{ Op::ADD8	  , false   , false , Adr::Immed8	, Reg::A    , CCOp::None, 2	 },
    /*8C*/  	{ Op::CMP16	  , false   , false , Adr::Immed16	, Reg::X    , CCOp::None, 4	 },
    /*8D*/  	{ Op::BSR	  , false   , false , Adr::Rel      , Reg::None , CCOp::None, 7	 },
    /*8E*/  	{ Op::LD16	  , false   , false , Adr::Immed16	, Reg::X    , CCOp::None, 3	 },
    /*8F*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*90*/  	{ Op::SUB8	  , false   , false , Adr::Direct	, Reg::A    , CCOp::None, 4	 },
    /*91*/  	{ Op::CMP8	  , false   , false , Adr::Direct	, Reg::A    , CCOp::None, 4	 },
    /*92*/  	{ Op::SBC	  , false   , false , Adr::Direct	, Reg::A    , CCOp::None, 4	 },
    /*93*/  	{ Op::SUB16	  , false   , false , Adr::Direct	, Reg::D    , CCOp::None, 6	 },
    /*94*/  	{ Op::AND	  , false   , false , Adr::Direct	, Reg::A    , CCOp::None, 4	 },
    /*95*/  	{ Op::BIT	  , false   , false , Adr::Direct	, Reg::A    , CCOp::None, 4	 },
    /*96*/  	{ Op::LD8	  , false   , false , Adr::Direct	, Reg::A    , CCOp::None, 4	 },
    /*97*/  	{ Op::ST8	  , false   , false , Adr::Direct	, Reg::A    , CCOp::None, 4	 },
    /*98*/  	{ Op::EOR	  , false   , false , Adr::Direct	, Reg::A    , CCOp::None, 4	 },
    /*99*/  	{ Op::ADC	  , false   , false , Adr::Direct	, Reg::A    , CCOp::None, 4	 },
    /*9A*/  	{ Op::OR	  , false   , false , Adr::Direct	, Reg::A    , CCOp::None, 4	 },
    /*9B*/  	{ Op::ADD8	  , false   , false , Adr::Direct	, Reg::A    , CCOp::None, 4	 },
    /*9C*/  	{ Op::CMP16	  , false   , false , Adr::Direct	, Reg::X    , CCOp::None, 6	 },
    /*9D*/  	{ Op::JSR	  , false   , false , Adr::Direct	, Reg::None , CCOp::None, 7	 },
    /*9E*/  	{ Op::LD16	  , false   , false , Adr::Direct	, Reg::X    , CCOp::None, 5	 },
    /*9F*/  	{ Op::ST16	  , false   , false , Adr::Direct	, Reg::X    , CCOp::None, 5	 },
    /*A0*/  	{ Op::SUB8	  , false   , false , Adr::Indexed	, Reg::A    , CCOp::None, 4	 },
    /*A1*/  	{ Op::CMP8	  , false   , false , Adr::Indexed	, Reg::A    , CCOp::None, 4	 },
    /*A2*/  	{ Op::SBC	  , false   , false , Adr::Indexed	, Reg::A    , CCOp::None, 4	 },
    /*A3*/  	{ Op::SUB16	  , false   , false , Adr::Indexed	, Reg::D    , CCOp::None, 6	 },
    /*A4*/  	{ Op::AND	  , false   , false , Adr::Indexed	, Reg::A    , CCOp::None, 4	 },
    /*A5*/  	{ Op::BIT	  , false   , false , Adr::Indexed	, Reg::A    , CCOp::None, 4	 },
    /*A6*/  	{ Op::LD8	  , false   , false , Adr::Indexed	, Reg::A    , CCOp::None, 4	 },
    /*A7*/  	{ Op::ST8	  , false   , false , Adr::Indexed	, Reg::A    , CCOp::None, 4	 },
    /*A8*/  	{ Op::EOR	  , false   , false , Adr::Indexed	, Reg::A    , CCOp::None, 4	 },
    /*A9*/  	{ Op::ADC	  , false   , false , Adr::Indexed	, Reg::A    , CCOp::None, 4	 },
    /*AA*/  	{ Op::OR	  , false   , false , Adr::Indexed	, Reg::A    , CCOp::None, 4	 },
    /*AB*/  	{ Op::ADD8	  , false   , false , Adr::Indexed	, Reg::A    , CCOp::None, 4	 },
    /*AC*/  	{ Op::CMP16	  , false   , false , Adr::Indexed	, Reg::X    , CCOp::None, 6	 },
    /*AD*/  	{ Op::JSR	  , false   , false , Adr::Indexed	, Reg::None , CCOp::None, 7	 },
    /*AE*/  	{ Op::LD16	  , false   , false , Adr::Indexed	, Reg::X    , CCOp::None, 5	 },
    /*AF*/  	{ Op::ST16	  , false   , false , Adr::Indexed	, Reg::X    , CCOp::None, 5	 },
    /*B0*/  	{ Op::SUB8	  , false   , false , Adr::Extended	, Reg::A    , CCOp::None, 5	 },
    /*B1*/  	{ Op::CMP8	  , false   , false , Adr::Extended	, Reg::A    , CCOp::None, 5	 },
    /*B2*/  	{ Op::SBC	  , false   , false , Adr::Extended	, Reg::A    , CCOp::None, 5	 },
    /*B3*/  	{ Op::SUB16	  , false   , false , Adr::Extended	, Reg::D    , CCOp::None, 7	 },
    /*B4*/  	{ Op::AND	  , false   , false , Adr::Extended	, Reg::A    , CCOp::None, 5	 },
    /*B5*/  	{ Op::BIT	  , false   , false , Adr::Extended	, Reg::A    , CCOp::None, 5	 },
    /*B6*/  	{ Op::LD8	  , false   , false , Adr::Extended	, Reg::A    , CCOp::None, 5	 },
    /*B7*/  	{ Op::ST8	  , false   , false , Adr::Extended	, Reg::A    , CCOp::None, 5	 },
    /*B8*/  	{ Op::EOR	  , false   , false , Adr::Extended	, Reg::A    , CCOp::None, 5	 },
    /*B9*/  	{ Op::ADC	  , false   , false , Adr::Extended	, Reg::A    , CCOp::None, 5	 },
    /*BA*/  	{ Op::OR	  , false   , false , Adr::Extended	, Reg::A    , CCOp::None, 5	 },
    /*BB*/  	{ Op::ADD8	  , false   , false , Adr::Extended	, Reg::A    , CCOp::None, 5	 },
    /*BC*/  	{ Op::CMP16	  , false   , false , Adr::Extended	, Reg::X    , CCOp::None, 7	 },
    /*BD*/  	{ Op::JSR	  , false   , false , Adr::Extended	, Reg::None , CCOp::None, 8	 },
    /*BE*/  	{ Op::LD16	  , false   , false , Adr::Extended	, Reg::X    , CCOp::None, 6	 },
    /*BF*/  	{ Op::ST16	  , false   , false , Adr::Extended	, Reg::X    , CCOp::None, 6	 },
    /*C0*/  	{ Op::SUB8	  , false   , false , Adr::Immed8	, Reg::B    , CCOp::None, 2	 },
    /*C1*/  	{ Op::CMP8	  , false   , false , Adr::Immed8	, Reg::B    , CCOp::None, 2	 },
    /*C2*/  	{ Op::SBC	  , false   , false , Adr::Immed8	, Reg::B    , CCOp::None, 2	 },
    /*C3*/  	{ Op::ADD16	  , false   , false , Adr::Immed16	, Reg::D    , CCOp::None, 4	 },
    /*C4*/  	{ Op::AND	  , false   , false , Adr::Immed8	, Reg::B    , CCOp::None, 2	 },
    /*C5*/  	{ Op::BIT	  , false   , false , Adr::Immed8	, Reg::B    , CCOp::None, 2	 },
    /*C6*/  	{ Op::LD8	  , false   , false , Adr::Immed8	, Reg::B    , CCOp::None, 2	 },
    /*C7*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*C8*/  	{ Op::EOR	  , false   , false , Adr::Immed8	, Reg::B    , CCOp::None, 2	 },
    /*C9*/  	{ Op::ADC	  , false   , false , Adr::Immed8	, Reg::B    , CCOp::None, 2	 },
    /*CA*/  	{ Op::OR	  , false   , false , Adr::Immed8	, Reg::B    , CCOp::None, 2	 },
    /*CB*/  	{ Op::ADD8	  , false   , false , Adr::Immed8	, Reg::B    , CCOp::None, 2	 },
    /*CC*/  	{ Op::LD16	  , false   , false , Adr::Immed16	, Reg::D    , CCOp::None, 3	 },
    /*CD*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*CE*/  	{ Op::LD16	  , false   , false , Adr::Immed16	, Reg::U    , CCOp::None, 3	 },
    /*CF*/  	{ Op::ILL	  , false   , false , Adr::None     , Reg::None , CCOp::None, 0  },
    /*D0*/  	{ Op::SUB8	  , false   , false , Adr::Direct	, Reg::B    , CCOp::None, 4	 },
    /*D1*/  	{ Op::CMP8	  , false   , false , Adr::Direct	, Reg::B    , CCOp::None, 4	 },
    /*D2*/  	{ Op::SBC	  , false   , false , Adr::Direct	, Reg::B    , CCOp::None, 4	 },
    /*D3*/  	{ Op::ADD16	  , false   , false , Adr::Direct	, Reg::D    , CCOp::None, 6	 },
    /*D4*/  	{ Op::AND	  , false   , false , Adr::Direct	, Reg::B    , CCOp::None, 4	 },
    /*D5*/  	{ Op::BIT	  , false   , false , Adr::Direct	, Reg::B    , CCOp::None, 4	 },
    /*D6*/  	{ Op::LD8	  , false   , false , Adr::Direct	, Reg::B    , CCOp::None, 4	 },
    /*D7*/  	{ Op::ST8	  , false   , false , Adr::Direct	, Reg::B    , CCOp::None, 4	 },
    /*D8*/  	{ Op::EOR	  , false   , false , Adr::Direct	, Reg::B    , CCOp::None, 4	 },
    /*D9*/  	{ Op::ADC	  , false   , false , Adr::Direct	, Reg::B    , CCOp::None, 4	 },
    /*DA*/  	{ Op::OR	  , false   , false , Adr::Direct	, Reg::B    , CCOp::None, 4	 },
    /*DB*/  	{ Op::ADD8	  , false   , false , Adr::Direct	, Reg::B    , CCOp::None, 4	 },
    /*DC*/  	{ Op::LD16	  , false   , false , Adr::Direct	, Reg::D    , CCOp::None, 5	 },
    /*DD*/  	{ Op::ST16	  , false   , false , Adr::Direct	, Reg::D    , CCOp::None, 5	 },
    /*DE*/  	{ Op::LD16	  , false   , false , Adr::Direct	, Reg::U    , CCOp::None, 5	 },
    /*DF*/  	{ Op::ST16	  , false   , false , Adr::Direct	, Reg::U    , CCOp::None, 5	 },
    /*E0*/  	{ Op::SUB8	  , false   , false , Adr::Indexed	, Reg::B    , CCOp::None, 4	 },
    /*E1*/  	{ Op::CMP8	  , false   , false , Adr::Indexed	, Reg::B    , CCOp::None, 4	 },
    /*E2*/  	{ Op::SBC	  , false   , false , Adr::Indexed	, Reg::B    , CCOp::None, 4	 },
    /*E3*/  	{ Op::ADD16	  , false   , false , Adr::Indexed	, Reg::D    , CCOp::None, 6	 },
    /*E4*/  	{ Op::AND	  , false   , false , Adr::Indexed	, Reg::B    , CCOp::None, 4	 },
    /*E5*/  	{ Op::BIT	  , false   , false , Adr::Indexed	, Reg::B    , CCOp::None, 4	 },
    /*E6*/  	{ Op::LD8	  , false   , false , Adr::Indexed	, Reg::B    , CCOp::None, 4	 },
    /*E7*/  	{ Op::ST8	  , false   , false , Adr::Indexed	, Reg::B    , CCOp::None, 4	 },
    /*E8*/  	{ Op::EOR	  , false   , false , Adr::Indexed	, Reg::B    , CCOp::None, 4	 },
    /*E9*/  	{ Op::ADC	  , false   , false , Adr::Indexed	, Reg::B    , CCOp::None, 4	 },
    /*EA*/  	{ Op::OR	  , false   , false , Adr::Indexed	, Reg::B    , CCOp::None, 4	 },
    /*EB*/  	{ Op::ADD8	  , false   , false , Adr::Indexed	, Reg::B    , CCOp::None, 4	 },
    /*EC*/  	{ Op::LD16	  , false   , false , Adr::Indexed	, Reg::D    , CCOp::None, 5	 },
    /*ED*/  	{ Op::ST16	  , false   , false , Adr::Indexed	, Reg::D    , CCOp::None, 5	 },
    /*EE*/  	{ Op::LD16	  , false   , false , Adr::Indexed	, Reg::U    , CCOp::None, 5	 },
    /*EF*/  	{ Op::ST16	  , false   , false , Adr::Indexed	, Reg::U    , CCOp::None, 5	 },
    /*F0*/  	{ Op::SUB8	  , false   , false , Adr::Extended , Reg::B    , CCOp::None, 5	 },
    /*F1*/  	{ Op::CMP8	  , false   , false , Adr::Extended , Reg::B    , CCOp::None, 5	 },
    /*F2*/  	{ Op::SBC	  , false   , false , Adr::Extended , Reg::B    , CCOp::None, 5	 },
    /*F3*/  	{ Op::ADD16	  , false   , false , Adr::Extended , Reg::D    , CCOp::None, 7	 },
    /*F4*/  	{ Op::AND	  , false   , false , Adr::Extended , Reg::B    , CCOp::None, 5	 },
    /*F5*/  	{ Op::BIT	  , false   , false , Adr::Extended , Reg::B    , CCOp::None, 5	 },
    /*F6*/  	{ Op::LD8	  , false   , false , Adr::Extended , Reg::B    , CCOp::None, 5	 },
    /*F7*/  	{ Op::ST8	  , false   , false , Adr::Extended , Reg::B    , CCOp::None, 5	 },
    /*F8*/  	{ Op::EOR	  , false   , false , Adr::Extended , Reg::B    , CCOp::None, 5	 },
    /*F9*/  	{ Op::ADC	  , false   , false , Adr::Extended , Reg::B    , CCOp::None, 5	 },
    /*FA*/  	{ Op::OR	  , false   , false , Adr::Extended , Reg::B    , CCOp::None, 5	 },
    /*FB*/  	{ Op::ADD8	  , false   , false , Adr::Extended , Reg::B    , CCOp::None, 5	 },
    /*FC*/  	{ Op::LD16	  , false   , false , Adr::Extended , Reg::D    , CCOp::None, 6	 },
    /*FD*/  	{ Op::ST16	  , false   , false , Adr::Extended , Reg::D    , CCOp::None, 6	 },
    /*FE*/  	{ Op::LD16	  , false   , false , Adr::Extended , Reg::U    , CCOp::None, 6	 },
    /*FF*/  	{ Op::ST16	  , false   , false , Adr::Extended , Reg::U    , CCOp::None, 6	 },
};

static_assert (sizeof(opcodeTable) == 256 * sizeof(Opcode), "Opcode table is wrong size");

static inline uint16_t concat(uint8_t a, uint8_t b)
{
    return (uint16_t(a) << 8) | uint16_t(b);
}

bool sim::execute(uint16_t addr)
{
    pc = addr;
    Op prevOp = Op::NOP;
    uint16_t ea = 0;
    uint32_t left = 0;
    uint32_t right = 0;
    uint32_t result = 0;
    
    while(true) {
        uint8_t opIndex = next8();
        
        const Opcode* opcode = &(opcodeTable[opIndex]);
                
        // Handle address modes
        // If this is an addressing mode that produces a 16 bit effective address
        // it will be placed in ea. If it's immediate or branch relative then the
        // 8 or 16 bit value is placed in right
        switch(opcode->adr) {
            case Adr::None:
            case Adr::Inherent:
                break;
            case Adr::Direct:
                ea = concat(dp, next8());
                break;
            case Adr::Extended:
                ea = next16();
                break;
            case Adr::Immed8:
                right = next8();
                break;
            case Adr::Immed16:
            case Adr::RelL:
                right = next16();
                break;
            case Adr::Rel:
                right = next8();
                break;
          case Adr::RelP:
                if (prevOp == Op::Page2) {
                    right = next16();
                    pc += 2;
                } else {
                    right = next8();
                }
                break;
            case Adr::Indexed: {
                uint8_t postbyte = next8();
                uint16_t* reg = nullptr;
                
                // Load value of RR reg in ea
                switch (RR(postbyte & 0b01100000)) {
                    case RR::X: reg = &x; break;
                    case RR::Y: reg = &y; break;
                    case RR::U: reg = &u; break;
                    case RR::S: reg = &s; break;
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
                        case IdxMode::ConstRegNoOff   : break;
                        case IdxMode::ConstReg8Off    : ea = *reg + int8_t(load8(pc)); pc += 1; break;
                        case IdxMode::ConstReg16Off   : ea = *reg + int16_t(load16(pc)); pc += 2; break;
                        case IdxMode::AccAOffReg      : ea = *reg + int8_t(a); break;
                        case IdxMode::AccBOffReg      : ea = *reg + int8_t(b); break;
                        case IdxMode::AccDOffReg      : ea = *reg + int16_t(d); break;
                        case IdxMode::Inc1Reg         : ea = *reg; (*reg) += 1; break;
                        case IdxMode::Inc2Reg         : ea = *reg; (*reg) += 2; break;
                        case IdxMode::Dec1Reg         : (*reg) += 1; ea = *reg; break;
                        case IdxMode::Dec2Reg         : (*reg) += 2; ea = *reg; break;
                        case IdxMode::ConstPC8Off     : ea = pc + int8_t(load8(pc)); pc += 1; break;
                        case IdxMode::ConstPC16Off    : ea = pc + int16_t(load16(pc)); pc += 2; break;
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
        if (opcode->loadLeft) {
            if (opcode->reg == Reg::M8) {
                left = load8(ea);
            } else if (opcode->reg == Reg::M16) {
                left = load16(ea);
            } else {
                left = getReg(opcode->reg);
            }
        }
                
        // Perform operation
        switch(opcode->op) {
            case Op::ILL:
                return false;
            case Op::Page2:
            case Op::Page3:
                break;

            case Op::BHS:
            case Op::BCC: if (!cc.C) pc += right; break;
            case Op::BLO:
            case Op::BCS: if (cc.C) pc += right; break;
            case Op::BEQ: if (cc.Z) pc += right; break;
            case Op::BGE: if ((cc.N ^ cc.V) == 0) pc += right; break;
            case Op::BGT: if ((cc.Z & (cc.N ^ cc.V)) == 0) pc += right; break;
            case Op::BHI: if ((cc.C | cc.Z) == 0) pc += right; break;
            case Op::BLE: if ((cc.Z | (cc.N ^ cc.V)) == 0) pc += right; break;
            case Op::BLS: if (cc.C || cc.Z) pc += right; break;
            case Op::BLT: if ((cc.N ^ cc.V) != 0) pc += right; break;
            case Op::BMI: if (cc.N) pc += right; break;
            case Op::BNE: if (!cc.Z) pc += right; break;
            case Op::BPL: if (!cc.N) pc += right; break;
            case Op::BRA: pc += right; break;
            case Op::BRN: break;
            case Op::BVC: if (!cc.V) pc += right; break;
            case Op::BVS: if (cc.V) pc += right; break;
            case Op::BSR:
                push16(s, pc);
                pc += right;
                break;

            case Op::ABX:
                x = x + uint16_t(b);
                break;
            case Op::ADC:
                result = left + right + (cc.C ? 1 : 0);
                break;
            case Op::ADD8:
                result = left + right;
                break;
            case Op::ADD16:
                result = left + right;
                break;
            case Op::AND:
                result = left & right;
                break;
            case Op::ANDCC:
                ccByte &= right;
                break;
            case Op::ASL:
                result = right << 1;
                break;
            case Op::ASR:
                result = int16_t(right) >> 1;
                break;
            case Op::BIT:
                result = left ^ right;
                break;
            case Op::CLR:
                result = 0;
                break;
            case Op::CMP8:
            case Op::SUB8:
                result = left - right;
                break;
            case Op::CMP16:
            case Op::SUB16:
                result = left - right;
                break;
            case Op::COM:
                result = ~right;
                break;
            case Op::CWAI:
                ccByte ^= right;
                cc.E = true;
                push16(s, pc);
                push16(s, u);
                push16(s, y);
                push16(s, x);
                push8(s, dp);
                push8(s, b);
                push8(s, a);
                
                // Now what?
                break;
            case Op::DAA:
                // LSN
                if (cc.H || (a & 0x0f) > 9) {
                    a += 6;
                }
                
                // MSN
                if (cc.C || (a & 0xf0) > 0x90) {
                    a += 0x60;
                }
                break;
            case Op::DEC:
                result = left - 1;
                break;
            case Op::EOR:
                result = left ^ right;
                break;
            case Op::EXG: {
                uint16_t r1 = getReg(Reg(right & 0xf));
                uint16_t r2 = getReg(Reg(right >> 4));
                setReg(Reg(right & 0xf), r2);
                setReg(Reg(right >> 4), r1);
                break;
            }
            case Op::INC:
                result = left + 1;
                break;
            case Op::JMP:
                pc = ea;
                break;
            case Op::JSR:
                push16(s, pc);
                pc = ea;
                break;
            case Op::LD8:
                result = load8(ea);
                break;
            case Op::LD16:
                result = load16(ea);
                break;
            case Op::LEA:
                result = ea;
                break;
            case Op::LSR:
                result = left >> 1;
                break;
            case Op::MUL:
                result = left * right;
                break;
            case Op::NEG:
                result = -left;
                break;
            case Op::NOP:
                break;
            case Op::OR:
                result = left | right;
                break;
            case Op::ORCC:
                ccByte |= right;
                break;
            case Op::PSH:
            case Op::PUL:
                break; // How do we deal with the postbyte bits and which stack?
            case Op::ROL:
                result = left << 1;
                if (cc.C) {
                    result |= 0x01;
                }
                break;
            case Op::ROR:
                result = left >> 1;
                if (cc.C) {
                    result |= 0x80;
                }
                break;
            case Op::RTI:
                if (cc.E) {
                    a = pop8(s);
                    b = pop8(s);
                    dp = pop8(s);
                    x = pop16(s);
                    y = pop16(s);
                    u = pop16(s);
                }
                pc = pop16(s);
                break;
            case Op::RTS:
                pc = pop16(s);
                break;
            case Op::SBC:
                result = left - right - (cc.C ? 1 : 0);
                break;
            case Op::SEX:
                a = (b & 0x80) ? 0xff : 0;
                break;
            case Op::ST8:
                store8(ea, left);
                break;
            case Op::ST16:
                store16(ea, left);
                break;
            case Op::SWI:
            case Op::SWI2:
            case Op::SWI3:
                cc.E = true;
                push16(s, pc);
                push16(s, u);
                push16(s, y);
                push16(s, x);
                push8(s, dp);
                push8(s, b);
                push8(s, a);
                cc.I = true;
                cc.F = true;
                switch(opcode->op) {
                    case Op::SWI: pc = load16(0xfffa); break;
                    case Op::SWI2: pc = load16(0xfff4); break;
                    case Op::SWI3: pc = load16(0xfff2); break;
                    default: break;
                }
                break;
            case Op::SYNC:
                // Now what?
                break;
            case Op::TFR:
                setReg(Reg(right >> 4), getReg(Reg(right & 0xf)));
                break;
            case Op::TST:
                result = left - 0;
                break;
            case Op::FIRQ:
            case Op::IRQ:
            case Op::NMI:
            case Op::RESTART:
                // Now what?
                break;
        }
        
        // Store result
        
        prevOp = opcode->op;
    }
    
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
