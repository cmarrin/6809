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

#ifdef COMPUTE_CYCLES
static_assert (sizeof(Opcode) == 4, "Opcode is wrong size");
#else
static_assert (sizeof(Opcode) == 3, "Opcode is wrong size");
#endif

static constexpr Opcode opcodeTable[ ] = {
    /*00*/  	{ Op::NEG	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	, CY(6) },
    /*01*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*02*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*03*/  	{ Op::COM	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	, CY(6) },
    /*04*/  	{ Op::LSR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	, CY(6) },
    /*05*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*06*/  	{ Op::ROR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	, CY(6) },
    /*07*/  	{ Op::ASR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	, CY(6) },
    /*08*/  	{ Op::ASL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	, CY(6) },
    /*09*/  	{ Op::ROL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	, CY(6) },
    /*0A*/  	{ Op::DEC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	, CY(6) },
    /*0B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*0C*/  	{ Op::INC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Direct	, CY(6) },
    /*0D*/  	{ Op::TST	  , Reg::M8   , Left::Ld  , Right::None , Adr::Direct	, CY(6) },
    /*0E*/  	{ Op::JMP	  , Reg::None , Left::None, Right::None , Adr::Direct	, CY(3) },
    /*0F*/  	{ Op::CLR	  , Reg::M8   , Left::St  , Right::None  ,Adr::Direct	, CY(6) },
    
    /*10*/  	{ Op::Page2	  , Reg::None , Left::None, Right::None , Adr::None	    , CY(0) },
    /*11*/  	{ Op::Page3	  , Reg::None , Left::None, Right::None , Adr::None	    , CY(0) },
    /*12*/  	{ Op::NOP	  , Reg::None , Left::None, Right::None , Adr::Inherent , CY(2) },
    /*13*/  	{ Op::SYNC	  , Reg::None , Left::None, Right::None , Adr::Inherent	, CY(4) },
    /*14*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*15*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*16*/  	{ Op::BRA	  , Reg::None , Left::None, Right::None , Adr::RelL	    , CY(5) },
    /*17*/  	{ Op::BSR	  , Reg::None , Left::None, Right::None , Adr::RelL	    , CY(9) },
    /*18*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*19*/  	{ Op::DAA	  , Reg::None , Left::None, Right::None , Adr::Inherent	, CY(2) },
    /*1A*/  	{ Op::ORCC	  , Reg::None , Left::None, Right::None , Adr::Immed8	, CY(3) },
    /*1B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*1C*/  	{ Op::ANDCC	  , Reg::None , Left::None, Right::None , Adr::Immed8	, CY(3) },
    /*1D*/  	{ Op::SEX	  , Reg::None , Left::None, Right::None , Adr::Inherent	, CY(2) },
    /*1E*/  	{ Op::EXG	  , Reg::None , Left::None, Right::None , Adr::Immed8	, CY(8) },
    /*1F*/  	{ Op::TFR	  , Reg::None , Left::None, Right::None , Adr::Immed8	, CY(6) },
    
    /*20*/  	{ Op::BRA	  , Reg::None , Left::None, Right::None , Adr::Rel	    , CY(3) },
    /*21*/  	{ Op::BRN	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*22*/  	{ Op::BHI	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*23*/  	{ Op::BLS	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*24*/  	{ Op::BHS	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*25*/  	{ Op::BLO	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*26*/  	{ Op::BNE	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*27*/  	{ Op::BEQ	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*28*/  	{ Op::BVC	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*29*/  	{ Op::BVS	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*2A*/  	{ Op::BPL	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*2B*/  	{ Op::BMI	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*2C*/  	{ Op::BGE	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*2D*/  	{ Op::BLT	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*2E*/  	{ Op::BGT	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    /*2F*/  	{ Op::BLE	  , Reg::None , Left::None, Right::None , Adr::RelP	    , CY(3) },
    
    /*30*/  	{ Op::LEA	  , Reg::X    , Left::St  , Right::None , Adr::Indexed	, CY(4) },
    /*31*/  	{ Op::LEA	  , Reg::Y    , Left::St,   Right::None , Adr::Indexed	, CY(4) },
    /*32*/  	{ Op::LEA	  , Reg::S    , Left::St,   Right::None , Adr::Indexed	, CY(4) },
    /*33*/  	{ Op::LEA	  , Reg::U    , Left::St,   Right::None , Adr::Indexed	, CY(4) },
    /*34*/  	{ Op::PSH	  , Reg::S    , Left::None, Right::None , Adr::Immed8	, CY(5) },
    /*35*/  	{ Op::PUL	  , Reg::S    , Left::None, Right::None , Adr::Immed8	, CY(5) },
    /*36*/  	{ Op::PSH	  , Reg::U    , Left::None, Right::None , Adr::Immed8	, CY(5) },
    /*37*/  	{ Op::PUL	  , Reg::U    , Left::None, Right::None , Adr::Immed8	, CY(5) },
    /*38*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    , CY(0) },
    /*39*/  	{ Op::RTS	  , Reg::None , Left::None, Right::None , Adr::Inherent	, CY(5) },
    /*3A*/  	{ Op::ABX	  , Reg::None , Left::None, Right::None , Adr::Inherent	, CY(3) },
    /*3B*/  	{ Op::RTI	  , Reg::None , Left::None, Right::None , Adr::Inherent	, CY(6) },          // 6 if FIRQ, 15 if IRQ
    /*3C*/  	{ Op::CWAI	  , Reg::None , Left::None, Right::None , Adr::Inherent	, CY(20)},
    /*3D*/  	{ Op::MUL	  , Reg::None , Left::None, Right::None , Adr::Inherent	, CY(11)},
    /*3E*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    , CY(0) },
    /*3F*/  	{ Op::SWI	  , Reg::None , Left::None, Right::None , Adr::Inherent	, CY(19)},
    
    /*40*/  	{ Op::NEG	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*41*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    , CY(0) },
    /*42*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*43*/  	{ Op::COM	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*44*/  	{ Op::LSR	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*45*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*46*/  	{ Op::ROR	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*47*/  	{ Op::ASR	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*48*/  	{ Op::ASL	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*49*/  	{ Op::ROL	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*4A*/  	{ Op::DEC	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*4B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*4C*/  	{ Op::INC	  , Reg::A    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*4D*/  	{ Op::TST	  , Reg::A    , Left::Ld  , Right::None , Adr::Inherent	, CY(2) },
    /*4E*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    , CY(0) },
    /*4F*/  	{ Op::CLR	  , Reg::A    , Left::St  , Right::None , Adr::Inherent	, CY(2) },
    
    /*50*/  	{ Op::NEG	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*51*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    , CY(0) },
    /*52*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*53*/  	{ Op::COM	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*54*/  	{ Op::LSR	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*55*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*56*/  	{ Op::ROR	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*57*/  	{ Op::ASR	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*58*/  	{ Op::ASL	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*59*/  	{ Op::ROL	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*5A*/  	{ Op::DEC	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*5B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*5C*/  	{ Op::INC	  , Reg::B    , Left::LdSt, Right::None , Adr::Inherent	, CY(2) },
    /*5D*/  	{ Op::TST	  , Reg::B    , Left::Ld  , Right::None , Adr::Inherent	, CY(2) },
    /*5E*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    , CY(0) },
    /*5F*/  	{ Op::CLR	  , Reg::B    , Left::St  , Right::None , Adr::Inherent	, CY(2) },
    
    /*60*/  	{ Op::NEG	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	, CY(6) },
    /*61*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    , CY(0) },
    /*62*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*63*/  	{ Op::COM	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	, CY(6) },
    /*64*/  	{ Op::LSR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	, CY(6) },
    /*65*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*66*/  	{ Op::ROR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	, CY(6) },
    /*67*/  	{ Op::ASR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	, CY(6) },
    /*68*/  	{ Op::ASL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	, CY(6) },
    /*69*/  	{ Op::ROL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	, CY(6) },
    /*6A*/  	{ Op::DEC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	, CY(6) },
    /*6B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*6C*/  	{ Op::INC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Indexed	, CY(6) },
    /*6D*/  	{ Op::TST	  , Reg::M8   , Left::Ld  , Right::None , Adr::Indexed	, CY(6) },
    /*6E*/  	{ Op::JMP	  , Reg::None , Left::None, Right::None , Adr::Indexed	, CY(3) },
    /*6F*/  	{ Op::CLR	  , Reg::M8   , Left::St  , Right::None , Adr::Indexed	, CY(6) },
    
    /*70*/  	{ Op::NEG	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	, CY(7) },
    /*71*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None	    , CY(0) },
    /*72*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*73*/  	{ Op::COM	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	, CY(7) },
    /*74*/  	{ Op::LSR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	, CY(7) },
    /*75*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*76*/  	{ Op::ROR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	, CY(7) },
    /*77*/  	{ Op::ASR	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	, CY(7) },
    /*78*/  	{ Op::ASL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	, CY(7) },
    /*79*/  	{ Op::ROL	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	, CY(7) },
    /*7A*/  	{ Op::DEC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	, CY(7) },
    /*7B*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*7C*/  	{ Op::INC	  , Reg::M8   , Left::LdSt, Right::None , Adr::Extended	, CY(7) },
    /*7D*/  	{ Op::TST	  , Reg::M8   , Left::Ld  , Right::None , Adr::Extended	, CY(7) },
    /*7E*/  	{ Op::JMP	  , Reg::None , Left::None, Right::None , Adr::Extended	, CY(4) },
    /*7F*/  	{ Op::CLR	  , Reg::M8   , Left::St  , Right::None , Adr::Extended	, CY(7) },
    
    /*80*/  	{ Op::SUB8	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*81*/  	{ Op::CMP8	  , Reg::A    , Left::Ld  , Right::None , Adr::Immed8	, CY(2) },
    /*82*/  	{ Op::SBC	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*83*/  	{ Op::SUB16	  , Reg::DDU  , Left::Ld  , Right::None , Adr::Immed16	, CY(4) },
    /*84*/  	{ Op::AND	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*85*/  	{ Op::BIT	  , Reg::A    , Left::Ld  , Right::None , Adr::Immed8	, CY(2) },
    /*86*/  	{ Op::LD8	  , Reg::A    , Left::St  , Right::None , Adr::Immed8	, CY(2) },
    /*87*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*88*/  	{ Op::EOR	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*89*/  	{ Op::ADC	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*8A*/  	{ Op::OR	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*8B*/  	{ Op::ADD8	  , Reg::A    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*8C*/  	{ Op::CMP16	  , Reg::XYS  , Left::Ld  , Right::None , Adr::Immed16	, CY(4) },
    /*8D*/  	{ Op::BSR	  , Reg::None , Left::None, Right::None , Adr::Rel      , CY(7) },
    /*8E*/  	{ Op::LD16	  , Reg::XY   , Left::St  , Right::None , Adr::Immed16	, CY(3) },
    /*8F*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    
    /*90*/  	{ Op::SUB8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*91*/  	{ Op::CMP8	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Direct	, CY(4) },
    /*92*/  	{ Op::SBC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*93*/  	{ Op::SUB16	  , Reg::DDU  , Left::Ld  , Right::Ld16 , Adr::Direct	, CY(6) },
    /*94*/  	{ Op::AND	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*95*/  	{ Op::BIT	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Direct	, CY(4) },
    /*96*/  	{ Op::LD8	  , Reg::A    , Left::St  , Right::Ld8  , Adr::Direct	, CY(4) },
    /*97*/  	{ Op::ST8	  , Reg::A    , Left::Ld  , Right::St8  , Adr::Direct	, CY(4) },
    /*98*/  	{ Op::EOR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*99*/  	{ Op::ADC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*9A*/  	{ Op::OR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*9B*/  	{ Op::ADD8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*9C*/  	{ Op::CMP16	  , Reg::X    , Left::Ld  , Right::Ld8  , Adr::Direct	, CY(6) },
    /*9D*/  	{ Op::JSR	  , Reg::None , Left::None, Right::None , Adr::Direct	, CY(7) },
    /*9E*/  	{ Op::LD16	  , Reg::XY   , Left::St  , Right::Ld16 , Adr::Direct	, CY(5) },
    /*9F*/  	{ Op::ST16	  , Reg::XY   , Left::Ld  , Right::St16 , Adr::Direct	, CY(5) },
    
    /*A0*/  	{ Op::SUB8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*A1*/  	{ Op::CMP8	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Indexed	, CY(4) },
    /*A2*/  	{ Op::SBC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*A3*/  	{ Op::SUB16	  , Reg::DDU  , Left::Ld  , Right::Ld16 , Adr::Indexed	, CY(6) },
    /*A4*/  	{ Op::AND	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*A5*/  	{ Op::BIT	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Indexed	, CY(4) },
    /*A6*/  	{ Op::LD8	  , Reg::A    , Left::St  , Right::Ld8  , Adr::Indexed	, CY(4) },
    /*A7*/  	{ Op::ST8	  , Reg::A    , Left::Ld  , Right::St8  , Adr::Indexed	, CY(4) },
    /*A8*/  	{ Op::EOR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*A9*/  	{ Op::ADC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*AA*/  	{ Op::OR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*AB*/  	{ Op::ADD8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*AC*/  	{ Op::CMP16	  , Reg::XYS  , Left::Ld  , Right::Ld16 , Adr::Indexed	, CY(6) },
    /*AD*/  	{ Op::JSR	  , Reg::None , Left::None, Right::None , Adr::Indexed	, CY(7) },
    /*AE*/  	{ Op::LD16	  , Reg::XY   , Left::St  , Right::Ld16 , Adr::Indexed	, CY(5) },
    /*AF*/  	{ Op::ST16	  , Reg::XY   , Left::Ld  , Right::St16 , Adr::Indexed	, CY(5) },
    
    /*B0*/  	{ Op::SUB8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	, CY(5) },
    /*B1*/  	{ Op::CMP8	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Extended	, CY(5) },
    /*B2*/  	{ Op::SBC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	, CY(5) },
    /*B3*/  	{ Op::SUB16	  , Reg::DDU  , Left::Ld  , Right::Ld16 , Adr::Extended	, CY(7) },
    /*B4*/  	{ Op::AND	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	, CY(5) },
    /*B5*/  	{ Op::BIT	  , Reg::A    , Left::Ld  , Right::Ld8  , Adr::Extended	, CY(5) },
    /*B6*/  	{ Op::LD8	  , Reg::A    , Left::St  , Right::Ld8  , Adr::Extended	, CY(5) },
    /*B7*/  	{ Op::ST8	  , Reg::A    , Left::Ld  , Right::St8  , Adr::Extended	, CY(5) },
    /*B8*/  	{ Op::EOR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	, CY(5) },
    /*B9*/  	{ Op::ADC	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	, CY(5) },
    /*BA*/  	{ Op::OR	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	, CY(5) },
    /*BB*/  	{ Op::ADD8	  , Reg::A    , Left::LdSt, Right::Ld8  , Adr::Extended	, CY(5) },
    /*BC*/  	{ Op::CMP16	  , Reg::XYS  , Left::Ld  , Right::Ld16 , Adr::Extended	, CY(7) },
    /*BD*/  	{ Op::JSR	  , Reg::None , Left::None, Right::None , Adr::Extended	, CY(8) },
    /*BE*/  	{ Op::LD16	  , Reg::XY   , Left::St  , Right::Ld16 , Adr::Extended	, CY(6) },
    /*BF*/  	{ Op::ST16	  , Reg::XY   , Left::Ld  , Right::St16 , Adr::Extended	, CY(6) },
    
    /*C0*/  	{ Op::SUB8	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*C1*/  	{ Op::CMP8	  , Reg::B    , Left::Ld  , Right::None , Adr::Immed8	, CY(2) },
    /*C2*/  	{ Op::SBC	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*C3*/  	{ Op::ADD16	  , Reg::D    , Left::LdSt, Right::None , Adr::Immed16	, CY(4) },
    /*C4*/  	{ Op::AND	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*C5*/  	{ Op::BIT	  , Reg::B    , Left::Ld  , Right::None , Adr::Immed8	, CY(2) },
    /*C6*/  	{ Op::LD8	  , Reg::B    , Left::St  , Right::None , Adr::Immed8	, CY(2) },
    /*C7*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*C8*/  	{ Op::EOR	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*C9*/  	{ Op::ADC	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*CA*/  	{ Op::OR	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*CB*/  	{ Op::ADD8	  , Reg::B    , Left::LdSt, Right::None , Adr::Immed8	, CY(2) },
    /*CC*/  	{ Op::LD16	  , Reg::D    , Left::St  , Right::None , Adr::Immed16	, CY(3) },
    /*CD*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    /*CE*/  	{ Op::LD16	  , Reg::US   , Left::St  , Right::None , Adr::Immed16	, CY(3) },
    /*CF*/  	{ Op::ILL	  , Reg::None , Left::None, Right::None , Adr::None     , CY(0) },
    
    /*D0*/  	{ Op::SUB8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*D1*/  	{ Op::CMP8	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Direct	, CY(4) },
    /*D2*/  	{ Op::SBC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*D3*/  	{ Op::ADD16	  , Reg::D    , Left::LdSt, Right::Ld16 , Adr::Direct	, CY(6) },
    /*D4*/  	{ Op::AND	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*D5*/  	{ Op::BIT	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Direct	, CY(4) },
    /*D6*/  	{ Op::LD8	  , Reg::B    , Left::St  , Right::Ld8  , Adr::Direct	, CY(4) },
    /*D7*/  	{ Op::ST8	  , Reg::B    , Left::Ld  , Right::St8  , Adr::Direct	, CY(4) },
    /*D8*/  	{ Op::EOR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*D9*/  	{ Op::ADC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*DA*/  	{ Op::OR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*DB*/  	{ Op::ADD8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Direct	, CY(4) },
    /*DC*/  	{ Op::LD16	  , Reg::D    , Left::St  , Right::Ld16 , Adr::Direct	, CY(5) },
    /*DD*/  	{ Op::ST16	  , Reg::D    , Left::Ld  , Right::St16 , Adr::Direct	, CY(5) },
    /*DE*/  	{ Op::LD16	  , Reg::US   , Left::St  , Right::Ld16 , Adr::Direct	, CY(5) },
    /*DF*/  	{ Op::ST16	  , Reg::US   , Left::Ld  , Right::St16 , Adr::Direct	, CY(5) },
    
    /*E0*/  	{ Op::SUB8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*E1*/  	{ Op::CMP8	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Indexed	, CY(4) },
    /*E2*/  	{ Op::SBC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*E3*/  	{ Op::ADD16	  , Reg::D    , Left::LdSt, Right::Ld16 , Adr::Indexed	, CY(6) },
    /*E4*/  	{ Op::AND	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*E5*/  	{ Op::BIT	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Indexed	, CY(4) },
    /*E6*/  	{ Op::LD8	  , Reg::B    , Left::St  , Right::Ld8  , Adr::Indexed	, CY(4) },
    /*E7*/  	{ Op::ST8	  , Reg::B    , Left::Ld  , Right::St8  , Adr::Indexed	, CY(4) },
    /*E8*/  	{ Op::EOR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*E9*/  	{ Op::ADC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*EA*/  	{ Op::OR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*EB*/  	{ Op::ADD8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Indexed	, CY(4) },
    /*EC*/  	{ Op::LD16	  , Reg::D    , Left::St  , Right::Ld16 , Adr::Indexed	, CY(5) },
    /*ED*/  	{ Op::ST16	  , Reg::D    , Left::Ld  , Right::St16 , Adr::Indexed	, CY(5) },
    /*EE*/  	{ Op::LD16	  , Reg::US   , Left::St  , Right::Ld16 , Adr::Indexed	, CY(5) },
    /*EF*/  	{ Op::ST16	  , Reg::US   , Left::Ld  , Right::St16 , Adr::Indexed	, CY(5) },
    
    /*F0*/  	{ Op::SUB8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended , CY(5) },
    /*F1*/  	{ Op::CMP8	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Extended , CY(5) },
    /*F2*/  	{ Op::SBC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended , CY(5) },
    /*F3*/  	{ Op::ADD16	  , Reg::D    , Left::LdSt, Right::Ld16 , Adr::Extended , CY(7) },
    /*F4*/  	{ Op::AND	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended , CY(5) },
    /*F5*/  	{ Op::BIT	  , Reg::B    , Left::Ld  , Right::Ld8  , Adr::Extended , CY(5) },
    /*F6*/  	{ Op::LD8	  , Reg::B    , Left::St  , Right::Ld8  , Adr::Extended , CY(5) },
    /*F7*/  	{ Op::ST8	  , Reg::B    , Left::Ld  , Right::St8  , Adr::Extended , CY(5) },
    /*F8*/  	{ Op::EOR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended , CY(5) },
    /*F9*/  	{ Op::ADC	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended , CY(5) },
    /*FA*/  	{ Op::OR	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended , CY(5) },
    /*FB*/  	{ Op::ADD8	  , Reg::B    , Left::LdSt, Right::Ld8  , Adr::Extended , CY(5) },
    /*FC*/  	{ Op::LD16	  , Reg::D    , Left::St  , Right::Ld16 , Adr::Extended , CY(6) },
    /*FD*/  	{ Op::ST16	  , Reg::D    , Left::Ld  , Right::St16 , Adr::Extended , CY(6) },
    /*FE*/  	{ Op::LD16	  , Reg::US   , Left::St  , Right::Ld16 , Adr::Extended , CY(6) },
    /*FF*/  	{ Op::ST16	  , Reg::US   , Left::Ld  , Right::St16 , Adr::Extended , CY(6) },
};

const Opcode*
Emulator::opcode(uint8_t i) { return opcodeTable + i; }

static_assert (sizeof(opcodeTable) == 256 * sizeof(Opcode), "Opcode table is wrong size");

static inline uint16_t concat(uint8_t a, uint8_t b)
{
    return (uint16_t(a) << 8) | uint16_t(b);
}

void SRecordInfo::ParseError(unsigned linenum, const char *fmt, va_list args)
{
    if (linenum == 0) {
        _boss9->printF("Error: line %d: ", linenum);
    } else {
        _boss9->printF("Error: ");
    }
    _boss9->vprintF(fmt, args);
    _boss9->printF("\n");
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
                _boss9->printF("\n*** hit breakpoint at addr $%04x\n\n", _pc);
                _boss9->call(Func::mon);
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

        AddCy(opcode->cycles);
                
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
                    AddCy(1);
                
                    int8_t offset = postbyte & 0x1f;
                    if (offset & 0x10) {
                        offset |= 0xe0;
                    }
                    
                    ea = *reg + offset;
                } else {
                    switch(IdxMode(postbyte & IdxModeMask)) {
                        case IdxMode::ConstRegNoOff   : ea = *reg; break;
                        case IdxMode::ConstReg8Off    : ea = *reg + int8_t(load8(_pc)); _pc += 1; AddCy(1); break;
                        case IdxMode::ConstReg16Off   : ea = *reg + int16_t(load16(_pc)); _pc += 2; AddCy(4); break;
                        case IdxMode::AccAOffReg      : ea = *reg + int8_t(_a); AddCy(1); break;
                        case IdxMode::AccBOffReg      : ea = *reg + int8_t(_b); AddCy(1); break;
                        case IdxMode::AccDOffReg      : ea = *reg + int16_t(_d); AddCy(4); break;
                        case IdxMode::Inc1Reg         : ea = *reg; (*reg) += 1; AddCy(2); break;
                        case IdxMode::Inc2Reg         : ea = *reg; (*reg) += 2; AddCy(3); break;
                        case IdxMode::Dec1Reg         : (*reg) -= 1; ea = *reg; AddCy(2); break;
                        case IdxMode::Dec2Reg         : (*reg) -= 2; ea = *reg; AddCy(3); break;
                        case IdxMode::ConstPC8Off     : ea = _pc + int8_t(load8(_pc)); _pc += 1; AddCy(1); break;
                        case IdxMode::ConstPC16Off    : ea = _pc + int16_t(load16(_pc)); _pc += 2; AddCy(5); break;
                        case IdxMode::Extended        : ea = next16(); AddCy(5); break;
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
                _left = getReg(opcode->reg);
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
                _boss9->call(Func::mon);
                return true;
            case Op::Page2:
            case Op::Page3:
                // We never want to leave execution after a Page2 or Page3.
                // Do a continue to skip leave test.
#ifdef COMPUTE_CYCLES
                _cycles += 1;
#endif
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
                _result = ~_left;
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
                    // This is possibly a system call. Push a dummy self and 
                    // retaddr then fixup U to keep the stack straight for args
                    push16(_s, 0);
                    push16(_s, 0);
                    push16(_s, _u);
                    _u = _s;
                    if (!_boss9->call(Func(ea))) {
                        pop16(_s);
                        pop16(_s);
                        return true;
                    }
                    _u = pop16(_s);
                    pop16(_s);
                    pop16(_s);
                } else {
                    if (op == Op::JSR) {
                        push16(_s, _pc);
                        _subroutineDepth += 1;
                    }
                    _pc = ea;
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
#ifdef COMPUTE_CYCLES
                _cycles += countBits(_right);
#endif
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
            case Op::ROR: {
                bool tmpC = _left & 0x01;
                _result = _left >> 1;
                xNZxx8();
                if (_cc.C) {
                    _result |= 0x80;
                }
                _cc.C = tmpC;
                break;
            }
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
                    _boss9->printF("\n*** step %s, stopped at addr $%04x\n\n",
                            (_lastRunState == RunState::StepOver) ? "over" : "out", _pc);
                    // enter the monitor
                    _boss9->call(Func::mon);
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
                setReg(opcode->reg, _result);
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
            _boss9->printF("\n*** step %s, stopped at addr $%04x\n\n",
                    (_lastRunState == RunState::StepIn) ? "in" : "over", _pc);
            _boss9->call(Func::mon);
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
    _boss9->printF("Address $%04x is read-only\n", addr);
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
