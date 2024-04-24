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

static_assert (sizeof(Opcode) == 3, "Opcode is wrong size");

static constexpr Opcode opcodeTable[ ] = {
    /*00*/  	{ Op::NEG	  , false   , Adr::Direct	    , Reg::None , 6     , 2 },
    /*01*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*02*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*03*/  	{ Op::COM	  , false   , Adr::Direct	    , Reg::None , 6	    , 2 },
    /*04*/  	{ Op::LSR	  , false   , Adr::Direct	    , Reg::None , 6	    , 2 },
    /*05*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*06*/  	{ Op::ROR	  , false   , Adr::Direct	    , Reg::None , 6	    , 2 },
    /*07*/  	{ Op::ASR	  , false   , Adr::Direct	    , Reg::None , 6	    , 2 },
    /*08*/  	{ Op::LSL	  , false   , Adr::Direct	    , Reg::None , 6	    , 2 },
    /*09*/  	{ Op::ROL	  , false   , Adr::Direct	    , Reg::None , 6	    , 2 },
    /*0A*/  	{ Op::DEC	  , false   , Adr::Direct	    , Reg::None , 6	    , 2 },
    /*0B*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*0C*/  	{ Op::INC	  , false   , Adr::Direct	    , Reg::None , 6	    , 2 },
    /*0D*/  	{ Op::TST	  , false   , Adr::Direct	    , Reg::None , 6	    , 2 },
    /*0E*/  	{ Op::JMP	  , false   , Adr::Direct	    , Reg::None , 3	    , 2 },
    /*0F*/  	{ Op::CLR	  , false   , Adr::Direct	    , Reg::None , 6	    , 2 },
    /*10*/  	{ Op::Page2	  , false   , Adr::None	    , Reg::None , 0	    , 0 },
    /*11*/  	{ Op::Page3	  , false   , Adr::None	    , Reg::None , 0	    , 0 },
    /*12*/  	{ Op::NOP	  , false   , Adr::Inherent   , Reg::None , 2	    , 1 },
    /*13*/  	{ Op::SYNC	  , false   , Adr::Inherent	, Reg::None , 4	    , 1 },
    /*14*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*15*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*16*/  	{ Op::BRA	  , false   , Adr::RelL	    , Reg::None , 5	    , 3 },
    /*17*/  	{ Op::BSR	  , false   , Adr::RelL	    , Reg::None , 9	    , 3 },
    /*18*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*19*/  	{ Op::DAA	  , false   , Adr::Inherent	, Reg::None , 2	    , 1 },
    /*1A*/  	{ Op::ORCC	  , false   , Adr::Immed8	    , Reg::None , 3	    , 2 },
    /*1B*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*1C*/  	{ Op::ANDCC	  , false   , Adr::Immed8	    , Reg::None , 3	    , 2 },
    /*1D*/  	{ Op::SEX	  , false   , Adr::Inherent	, Reg::None , 2	    , 1 },
    /*1E*/  	{ Op::EXG	  , false   , Adr::Immed8	    , Reg::None , 8	    , 2 },
    /*1F*/  	{ Op::TFR	  , false   , Adr::Immed8	    , Reg::None , 6	    , 2 },
    /*20*/  	{ Op::BRA	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*21*/  	{ Op::BRN	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*22*/  	{ Op::BHI	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*23*/  	{ Op::BLS	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*24*/  	{ Op::BHS	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*25*/  	{ Op::BLO	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*26*/  	{ Op::BNE	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*27*/  	{ Op::BEQ	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*28*/  	{ Op::BVC	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*29*/  	{ Op::BVS	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*2A*/  	{ Op::BPL	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*2B*/  	{ Op::BMI	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*2C*/  	{ Op::BGE	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*2D*/  	{ Op::BLT	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*2E*/  	{ Op::BGT	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*2F*/  	{ Op::BLE	  , false   , Adr::Rel	    , Reg::None , 3	    , 2 },
    /*30*/  	{ Op::LEA	  , false   , Adr::Indexed	, Reg::X    , 4	    , 2 },
    /*31*/  	{ Op::LEA	  , false   , Adr::Indexed	, Reg::Y    , 4	    , 2 },
    /*32*/  	{ Op::LEA	  , false   , Adr::Indexed	, Reg::S    , 4	    , 2 },
    /*33*/  	{ Op::LEA	  , false   , Adr::Indexed	, Reg::U    , 4	    , 2 },
    /*34*/  	{ Op::PSH	  , false   , Adr::Immed8	    , Reg::S    , 5	    , 2 },
    /*35*/  	{ Op::PUL	  , false   , Adr::Immed8	    , Reg::S    , 5	    , 2 },
    /*36*/  	{ Op::PSH	  , false   , Adr::Immed8	    , Reg::U    , 5	    , 2 },
    /*37*/  	{ Op::PUL	  , false   , Adr::Immed8	    , Reg::U    , 5	    , 2 },
    /*38*/  	{ Op::ILL	  , false   , Adr::None	    , Reg::None , 0	    , 0 },
    /*39*/  	{ Op::RTS	  , false   , Adr::Inherent	, Reg::None , 5	    , 1 },
    /*3A*/  	{ Op::ABX	  , false   , Adr::Inherent	, Reg::None , 3	    , 1 },
    /*3B*/  	{ Op::RTI	  , false   , Adr::Inherent	, Reg::None , 6	    , 1 },          // 6 if FIRQ, 15 if IRQ
    /*3C*/  	{ Op::CWAI	  , false   , Adr::Inherent	, Reg::None , 20	, 2 },
    /*3D*/  	{ Op::MUL	  , false   , Adr::Inherent	, Reg::None , 11	, 1 },
    /*3E*/  	{ Op::ILL	  , false   , Adr::None	    , Reg::None , 0	    , 0 },
    /*3F*/  	{ Op::SWI	  , false   , Adr::Inherent	, Reg::None , 19	, 1 },
    /*40*/  	{ Op::NEG	  , false   , Adr::Inherent	, Reg::A    , 2	    , 1 },
    /*41*/  	{ Op::ILL	  , false   , Adr::None	    , Reg::None , 0	    , 0 },
    /*42*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*43*/  	{ Op::COM	  , false   , Adr::Inherent	, Reg::A    , 2	    , 1 },
    /*44*/  	{ Op::LSR	  , false   , Adr::Inherent	, Reg::A    , 2	    , 1 },
    /*45*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*46*/  	{ Op::ROR	  , false   , Adr::Inherent	, Reg::A    , 2	    , 1 },
    /*47*/  	{ Op::ASR	  , false   , Adr::Inherent	, Reg::A    , 2	    , 1 },
    /*48*/  	{ Op::LSL	  , false   , Adr::Inherent	, Reg::A    , 2	    , 1 },
    /*49*/  	{ Op::ROL	  , false   , Adr::Inherent	, Reg::A    , 2	    , 1 },
    /*4A*/  	{ Op::DEC	  , false   , Adr::Inherent	, Reg::A    , 2	    , 1 },
    /*4B*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*4C*/  	{ Op::INC	  , false   , Adr::Inherent	, Reg::A    , 2	    , 1 },
    /*4D*/  	{ Op::TST	  , false   , Adr::Inherent	, Reg::A    , 2	    , 1 },
    /*4E*/  	{ Op::ILL	  , false   , Adr::None	    , Reg::None , 0	    , 0 },
    /*4F*/  	{ Op::CLR	  , false   , Adr::Inherent	, Reg::A    , 2	    , 1 },
    /*50*/  	{ Op::NEG	  , false   , Adr::Inherent	, Reg::B    , 2	    , 1 },
    /*51*/  	{ Op::ILL	  , false   , Adr::None	    , Reg::None , 0	    , 0 },
    /*52*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*53*/  	{ Op::COM	  , false   , Adr::Inherent	, Reg::B    , 2	    , 1 },
    /*54*/  	{ Op::LSR	  , false   , Adr::Inherent	, Reg::B    , 2	    , 1 },
    /*55*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*56*/  	{ Op::ROR	  , false   , Adr::Inherent	, Reg::B    , 2	    , 1 },
    /*57*/  	{ Op::ASR	  , false   , Adr::Inherent	, Reg::B    , 2	    , 1 },
    /*58*/  	{ Op::LSL	  , false   , Adr::Inherent	, Reg::B    , 2	    , 1 },
    /*59*/  	{ Op::ROL	  , false   , Adr::Inherent	, Reg::B    , 2	    , 1 },
    /*5A*/  	{ Op::DEC	  , false   , Adr::Inherent	, Reg::B    , 2	    , 1 },
    /*5B*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*5C*/  	{ Op::INC	  , false   , Adr::Inherent	, Reg::B    , 2	    , 1 },
    /*5D*/  	{ Op::TST	  , false   , Adr::Inherent	, Reg::B    , 2	    , 1 },
    /*5E*/  	{ Op::ILL	  , false   , Adr::None	    , Reg::None , 0	    , 0 },
    /*5F*/  	{ Op::CLR	  , false   , Adr::Inherent	, Reg::B    , 2	    , 1 },
    /*60*/  	{ Op::NEG	  , false   , Adr::Indexed	, Reg::None , 6	    , 2 },
    /*61*/  	{ Op::ILL	  , false   , Adr::None	    , Reg::None , 0	    , 0 },
    /*62*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*63*/  	{ Op::COM	  , false   , Adr::Indexed	, Reg::None , 6	    , 2 },
    /*64*/  	{ Op::LSR	  , false   , Adr::Indexed	, Reg::None , 6	    , 2 },
    /*65*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*66*/  	{ Op::ROR	  , false   , Adr::Indexed	, Reg::None , 6	    , 2 },
    /*67*/  	{ Op::ASR	  , false   , Adr::Indexed	, Reg::None , 6	    , 2 },
    /*68*/  	{ Op::LSL	  , false   , Adr::Indexed	, Reg::None , 6	    , 2 },
    /*69*/  	{ Op::ROL	  , false   , Adr::Indexed	, Reg::None , 6	    , 2 },
    /*6A*/  	{ Op::DEC	  , false   , Adr::Indexed	, Reg::None , 6	    , 2 },
    /*6B*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*6C*/  	{ Op::INC	  , false   , Adr::Indexed	, Reg::None , 6	    , 2 },
    /*6D*/  	{ Op::TST	  , false   , Adr::Indexed	, Reg::None , 6	    , 2 },
    /*6E*/  	{ Op::JMP	  , false   , Adr::Indexed	, Reg::None , 3	    , 2 },
    /*6F*/  	{ Op::CLR	  , false   , Adr::Indexed	, Reg::None , 6	    , 2 },
    /*70*/  	{ Op::NEG	  , false   , Adr::Extended	, Reg::None , 7	    , 3 },
    /*71*/  	{ Op::ILL	  , false   , Adr::None	    , Reg::None , 0	    , 0 },
    /*72*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*73*/  	{ Op::COM	  , false   , Adr::Extended	, Reg::None , 7	    , 3 },
    /*74*/  	{ Op::LSR	  , false   , Adr::Extended	, Reg::None , 7	    , 3 },
    /*75*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*76*/  	{ Op::ROR	  , false   , Adr::Extended	, Reg::None , 7	    , 3 },
    /*77*/  	{ Op::ASR	  , false   , Adr::Extended	, Reg::None , 7	    , 3 },
    /*78*/  	{ Op::LSL	  , false   , Adr::Extended	, Reg::None , 7	    , 3 },
    /*79*/  	{ Op::ROL	  , false   , Adr::Extended	, Reg::None , 7	    , 3 },
    /*7A*/  	{ Op::DEC	  , false   , Adr::Extended	, Reg::None , 7	    , 3 },
    /*7B*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*7C*/  	{ Op::INC	  , false   , Adr::Extended	, Reg::None , 7	    , 3 },
    /*7D*/  	{ Op::TST	  , false   , Adr::Extended	, Reg::None , 7	    , 3 },
    /*7E*/  	{ Op::JMP	  , false   , Adr::Extended	, Reg::None , 4	    , 3 },
    /*7F*/  	{ Op::CLR	  , false   , Adr::Extended	, Reg::None , 7	    , 3 },
    /*80*/  	{ Op::SUB8	  , false   , Adr::Immed8	    , Reg::A    , 2	    , 2 },
    /*81*/  	{ Op::CMP8	  , false   , Adr::Immed8	    , Reg::A    , 2	    , 2 },
    /*82*/  	{ Op::SBC	  , false   , Adr::Immed8	    , Reg::A    , 2	    , 2 },
    /*83*/  	{ Op::SUB16	  , false   , Adr::Immed16	    , Reg::D    , 4	    , 3 },
    /*84*/  	{ Op::AND	  , false   , Adr::Immed8	    , Reg::A    , 2	    , 2 },
    /*85*/  	{ Op::BIT	  , false   , Adr::Immed8	    , Reg::A    , 2	    , 2 },
    /*86*/  	{ Op::LD8	  , false   , Adr::Immed8	    , Reg::A    , 2	    , 2 },
    /*87*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0      , 0 },
    /*88*/  	{ Op::EOR	  , false   , Adr::Immed8	    , Reg::A    , 2	    , 2 },
    /*89*/  	{ Op::ADC	  , false   , Adr::Immed8	    , Reg::A    , 2	    , 2 },
    /*8A*/  	{ Op::OR	  , false   , Adr::Immed8	    , Reg::A    , 2	    , 2 },
    /*8B*/  	{ Op::ADD8	  , false   , Adr::Immed8	    , Reg::A    , 2	    , 2 },
    /*8C*/  	{ Op::CMP16	  , false   , Adr::Immed16	    , Reg::X    , 4	    , 3 },
    /*8D*/  	{ Op::BSR	  , false   , Adr::Rel        , Reg::None , 7	    , 2 },
    /*8E*/  	{ Op::LD16	  , false   , Adr::Immed16	    , Reg::X    , 3	    , 3 },
    /*8F*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0      , 0 },
    /*90*/  	{ Op::SUB8	  , false   , Adr::Direct	    , Reg::A    , 4	    , 2 },
    /*91*/  	{ Op::CMP8	  , false   , Adr::Direct	    , Reg::A    , 4	    , 2 },
    /*92*/  	{ Op::SBC	  , false   , Adr::Direct	    , Reg::A    , 4	    , 2 },
    /*93*/  	{ Op::SUB16	  , false   , Adr::Direct	    , Reg::D    , 6	    , 2 },
    /*94*/  	{ Op::AND	  , false   , Adr::Direct	    , Reg::A    , 4	    , 2 },
    /*95*/  	{ Op::BIT	  , false   , Adr::Direct	    , Reg::A    , 4	    , 2 },
    /*96*/  	{ Op::LD8	  , false   , Adr::Direct	    , Reg::A    , 4	    , 2 },
    /*97*/  	{ Op::ST8	  , false   , Adr::Direct	    , Reg::A    , 4	    , 2 },
    /*98*/  	{ Op::EOR	  , false   , Adr::Direct	    , Reg::A    , 4	    , 2 },
    /*99*/  	{ Op::ADC	  , false   , Adr::Direct	    , Reg::A    , 4	    , 2 },
    /*9A*/  	{ Op::OR	  , false   , Adr::Direct	    , Reg::A    , 4	    , 2 },
    /*9B*/  	{ Op::ADD8	  , false   , Adr::Direct	    , Reg::A    , 4	    , 2 },
    /*9C*/  	{ Op::CMP16	  , false   , Adr::Direct	    , Reg::X    , 6	    , 2 },
    /*9D*/  	{ Op::JSR	  , false   , Adr::Direct	    , Reg::None , 7	    , 2 },
    /*9E*/  	{ Op::LD16	  , false   , Adr::Direct	    , Reg::X    , 5	    , 2 },
    /*9F*/  	{ Op::ST16	  , false   , Adr::Direct	    , Reg::X    , 5	    , 2 },
    /*A0*/  	{ Op::SUB8	  , false   , Adr::Indexed	, Reg::A    , 4	    , 2 },
    /*A1*/  	{ Op::CMP8	  , false   , Adr::Indexed	, Reg::A    , 4	    , 2 },
    /*A2*/  	{ Op::SBC	  , false   , Adr::Indexed	, Reg::A    , 4	    , 2 },
    /*A3*/  	{ Op::SUB16	  , false   , Adr::Indexed	, Reg::D    , 6	    , 2 },
    /*A4*/  	{ Op::AND	  , false   , Adr::Indexed	, Reg::A    , 4	    , 2 },
    /*A5*/  	{ Op::BIT	  , false   , Adr::Indexed	, Reg::A    , 4	    , 2 },
    /*A6*/  	{ Op::LD8	  , false   , Adr::Indexed	, Reg::A    , 4	    , 2 },
    /*A7*/  	{ Op::ST8	  , false   , Adr::Indexed	, Reg::A    , 4	    , 2 },
    /*A8*/  	{ Op::EOR	  , false   , Adr::Indexed	, Reg::A    , 4	    , 2 },
    /*A9*/  	{ Op::ADC	  , false   , Adr::Indexed	, Reg::A    , 4	    , 2 },
    /*AA*/  	{ Op::OR	  , false   , Adr::Indexed	, Reg::A    , 4	    , 2 },
    /*AB*/  	{ Op::ADD8	  , false   , Adr::Indexed	, Reg::A    , 4	    , 2 },
    /*AC*/  	{ Op::CMP16	  , false   , Adr::Indexed	, Reg::X    , 6	    , 2 },
    /*AD*/  	{ Op::JSR	  , false   , Adr::Indexed	, Reg::None , 7	    , 2 },
    /*AE*/  	{ Op::LD16	  , false   , Adr::Indexed	, Reg::X    , 5	    , 2 },
    /*AF*/  	{ Op::ST16	  , false   , Adr::Indexed	, Reg::X    , 5	    , 2 },
    /*B0*/  	{ Op::SUB8	  , false   , Adr::Extended	, Reg::A    , 5	    , 3 },
    /*B1*/  	{ Op::CMP8	  , false   , Adr::Extended	, Reg::A    , 5	    , 3 },
    /*B2*/  	{ Op::SBC	  , false   , Adr::Extended	, Reg::A    , 5	    , 3 },
    /*B3*/  	{ Op::SUB16	  , false   , Adr::Extended	, Reg::D    , 7	    , 3 },
    /*B4*/  	{ Op::AND	  , false   , Adr::Extended	, Reg::A    , 5	    , 3 },
    /*B5*/  	{ Op::BIT	  , false   , Adr::Extended	, Reg::A    , 5	    , 3 },
    /*B6*/  	{ Op::LD8	  , false   , Adr::Extended	, Reg::A    , 5	    , 3 },
    /*B7*/  	{ Op::ST8	  , false   , Adr::Extended	, Reg::A    , 5	    , 3 },
    /*B8*/  	{ Op::EOR	  , false   , Adr::Extended	, Reg::A    , 5	    , 3 },
    /*B9*/  	{ Op::ADC	  , false   , Adr::Extended	, Reg::A    , 5	    , 3 },
    /*BA*/  	{ Op::OR	  , false   , Adr::Extended	, Reg::A    , 5	    , 3 },
    /*BB*/  	{ Op::ADD8	  , false   , Adr::Extended	, Reg::A    , 5	    , 3 },
    /*BC*/  	{ Op::CMP16	  , false   , Adr::Extended	, Reg::X    , 7	    , 3 },
    /*BD*/  	{ Op::JSR	  , false   , Adr::Extended	, Reg::None , 8	    , 3 },
    /*BE*/  	{ Op::LD16	  , false   , Adr::Extended	, Reg::X    , 6	    , 3 },
    /*BF*/  	{ Op::ST16	  , false   , Adr::Extended	, Reg::X    , 6	    , 3 },
    /*C0*/  	{ Op::SUB8	  , false   , Adr::Immed8	    , Reg::B    , 2	    , 2 },
    /*C1*/  	{ Op::CMP8	  , false   , Adr::Immed8	    , Reg::B    , 2	    , 2 },
    /*C2*/  	{ Op::SBC	  , false   , Adr::Immed8	    , Reg::B    , 2	    , 2 },
    /*C3*/  	{ Op::ADD16	  , false   , Adr::Immed16	    , Reg::D    , 4	    , 3 },
    /*C4*/  	{ Op::AND	  , false   , Adr::Immed8	    , Reg::B    , 2	    , 2 },
    /*C5*/  	{ Op::BIT	  , false   , Adr::Immed8	    , Reg::B    , 2	    , 2 },
    /*C6*/  	{ Op::LD8	  , false   , Adr::Immed8	    , Reg::B    , 2	    , 2 },
    /*C7*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*C8*/  	{ Op::EOR	  , false   , Adr::Immed8	    , Reg::B    , 2	    , 2 },
    /*C9*/  	{ Op::ADC	  , false   , Adr::Immed8	    , Reg::B    , 2	    , 2 },
    /*CA*/  	{ Op::OR	  , false   , Adr::Immed8	    , Reg::B    , 2	    , 2 },
    /*CB*/  	{ Op::ADD8	  , false   , Adr::Immed8	    , Reg::B    , 2	    , 2 },
    /*CC*/  	{ Op::LD16	  , false   , Adr::Immed16	    , Reg::D    , 3	    , 3 },
    /*CD*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*CE*/  	{ Op::LD16	  , false   , Adr::Immed16	    , Reg::U    , 3	    , 3 },
    /*CF*/  	{ Op::ILL	  , false   , Adr::None       , Reg::None , 0     , 0 },
    /*D0*/  	{ Op::SUB8	  , false   , Adr::Direct	    , Reg::B    , 4	    , 2 },
    /*D1*/  	{ Op::CMP8	  , false   , Adr::Direct	    , Reg::B    , 4	    , 2 },
    /*D2*/  	{ Op::SBC	  , false   , Adr::Direct	    , Reg::B    , 4	    , 2 },
    /*D3*/  	{ Op::ADD16	  , false   , Adr::Direct	    , Reg::D    , 6	    , 2 },
    /*D4*/  	{ Op::AND	  , false   , Adr::Direct	    , Reg::B    , 4	    , 2 },
    /*D5*/  	{ Op::BIT	  , false   , Adr::Direct	    , Reg::B    , 4	    , 2 },
    /*D6*/  	{ Op::LD8	  , false   , Adr::Direct	    , Reg::B    , 4	    , 2 },
    /*D7*/  	{ Op::ST8	  , false   , Adr::Direct	    , Reg::B    , 4	    , 2 },
    /*D8*/  	{ Op::EOR	  , false   , Adr::Direct	    , Reg::B    , 4	    , 2 },
    /*D9*/  	{ Op::ADC	  , false   , Adr::Direct	    , Reg::B    , 4	    , 2 },
    /*DA*/  	{ Op::OR	  , false   , Adr::Direct	    , Reg::B    , 4	    , 2 },
    /*DB*/  	{ Op::ADD8	  , false   , Adr::Direct	    , Reg::B    , 4	    , 2 },
    /*DC*/  	{ Op::LD16	  , false   , Adr::Direct	    , Reg::D    , 5	    , 2 },
    /*DD*/  	{ Op::ST16	  , false   , Adr::Direct	    , Reg::D    , 5	    , 2 },
    /*DE*/  	{ Op::LD16	  , false   , Adr::Direct	    , Reg::U    , 5	    , 2 },
    /*DF*/  	{ Op::ST16	  , false   , Adr::Direct	    , Reg::U    , 5	    , 2 },
    /*E0*/  	{ Op::SUB8	  , false   , Adr::Indexed	, Reg::B    , 4	    , 2 },
    /*E1*/  	{ Op::CMP8	  , false   , Adr::Indexed	, Reg::B    , 4	    , 2 },
    /*E2*/  	{ Op::SBC	  , false   , Adr::Indexed	, Reg::B    , 4	    , 2 },
    /*E3*/  	{ Op::ADD16	  , false   , Adr::Indexed	, Reg::D    , 6	    , 2 },
    /*E4*/  	{ Op::AND	  , false   , Adr::Indexed	, Reg::B    , 4	    , 2 },
    /*E5*/  	{ Op::BIT	  , false   , Adr::Indexed	, Reg::B    , 4	    , 2 },
    /*E6*/  	{ Op::LD8	  , false   , Adr::Indexed	, Reg::B    , 4	    , 2 },
    /*E7*/  	{ Op::ST8	  , false   , Adr::Indexed	, Reg::B    , 4	    , 2 },
    /*E8*/  	{ Op::EOR	  , false   , Adr::Indexed	, Reg::B    , 4	    , 2 },
    /*E9*/  	{ Op::ADC	  , false   , Adr::Indexed	, Reg::B    , 4	    , 2 },
    /*EA*/  	{ Op::OR	  , false   , Adr::Indexed	, Reg::B    , 4	    , 2 },
    /*EB*/  	{ Op::ADD8	  , false   , Adr::Indexed	, Reg::B    , 4	    , 2 },
    /*EC*/  	{ Op::LD16	  , false   , Adr::Indexed	, Reg::D    , 5	    , 2 },
    /*ED*/  	{ Op::ST16	  , false   , Adr::Indexed	, Reg::D    , 5	    , 2 },
    /*EE*/  	{ Op::LD16	  , false   , Adr::Indexed	, Reg::U    , 5	    , 2 },
    /*EF*/  	{ Op::ST16	  , false   , Adr::Indexed	, Reg::U    , 5	    , 2 },
    /*F0*/  	{ Op::SUB8	  , false   , Adr::Extended   , Reg::B    , 5	    , 3 },
    /*F1*/  	{ Op::CMP8	  , false   , Adr::Extended   , Reg::B    , 5	    , 3 },
    /*F2*/  	{ Op::SBC	  , false   , Adr::Extended   , Reg::B    , 5	    , 3 },
    /*F3*/  	{ Op::ADD16	  , false   , Adr::Extended   , Reg::D    , 7	    , 3 },
    /*F4*/  	{ Op::AND	  , false   , Adr::Extended   , Reg::B    , 5	    , 3 },
    /*F5*/  	{ Op::BIT	  , false   , Adr::Extended   , Reg::B    , 5	    , 3 },
    /*F6*/  	{ Op::LD8	  , false   , Adr::Extended   , Reg::B    , 5	    , 3 },
    /*F7*/  	{ Op::ST8	  , false   , Adr::Extended   , Reg::B    , 5	    , 3 },
    /*F8*/  	{ Op::EOR	  , false   , Adr::Extended   , Reg::B    , 5	    , 3 },
    /*F9*/  	{ Op::ADC	  , false   , Adr::Extended   , Reg::B    , 5	    , 3 },
    /*FA*/  	{ Op::OR	  , false   , Adr::Extended   , Reg::B    , 5	    , 3 },
    /*FB*/  	{ Op::ADD8	  , false   , Adr::Extended   , Reg::B    , 5	    , 3 },
    /*FC*/  	{ Op::LD16	  , false   , Adr::Extended   , Reg::D    , 6	    , 3 },
    /*FD*/  	{ Op::ST16	  , false   , Adr::Extended   , Reg::D    , 6	    , 3 },
    /*FE*/  	{ Op::LD16	  , false   , Adr::Extended   , Reg::U    , 6	    , 3 },
    /*FF*/  	{ Op::ST16	  , false   , Adr::Extended   , Reg::U    , 6	    , 3 },
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
        uint8_t opIndex = ram[pc];
        
        const Opcode* opcode = &(opcodeTable[opIndex]);
                
        // Handle address modes
        // If this is an addressing mode that produces a 16 bit effective address
        // it will be placed in ea. If it's immediate or branch relative then the
        // 8 or 16 bit value is placed in right
        switch(opcode->adr) {
            case Adr::None:
                break;
            case Adr::Direct:
                ea = concat(dp, ram[pc++]);
                break;
            case Adr::Immed8:
                right = ram[pc++];
                break;
            case Adr::Immed16:
            case Adr::RelL:
                right = concat(ram[pc], ram[pc + 1]);
                pc += 2;
                break;
            case Adr::Rel:
                if (prevOp == Op::Page2) {
                    right = concat(ram[pc], ram[pc + 1]);
                    pc += 2;
                } else {
                    right = ram[pc++];
                }
                break;
            case Adr::Indexed:
            case Adr::Extended:
            case Adr::Special:
            case Adr::Inherent:
                break;
        }
        
        // Get left operand
        if (opcode->loadLeft) {
            switch(opcode->reg) {
                case Reg::None: break;
                case Reg::A:    left = a; break;
                case Reg::B:    left = b; break;
                case Reg::D:    left = d; break;
                case Reg::X:    left = x; break;
                case Reg::Y:    left = y; break;
                case Reg::U:    left = u; break;
                case Reg::S:    left = s; break;
                case Reg::PC:
                case Reg::DP:
                case Reg::CC:   break;
            }
        }
                
        // Perform operation
        switch(opcode->op) {
            case Op::ILL:
                return false;
            case Op::Page2:
            case Op::Page3:
                break;
            case Op::ABX:
                x = x + uint16_t(b);
                break;
            case Op::ADC:
                result = left + right + (cc.C ? 1 : 0);
                updateCC8(left, right, result, true);
                break;
            case Op::ADD8:
                result = left + right;
                updateCC8(left, right, result, true);
                break;
            case Op::ADD16:
                result = left + right;
                updateCC16(left, right, result);
                break;
            case Op::AND:
            case Op::ANDCC:
            case Op::ASR:
            case Op::BCC:
            case Op::BCS:
            case Op::BEQ:
            case Op::BGE:
            case Op::BGT:
            case Op::BHI:
            case Op::BHS:
            case Op::BIT:
            case Op::BLE:
            case Op::BLO:
            case Op::BLS:
            case Op::BLT:
            case Op::BMI:
            case Op::BNE:
            case Op::BPL:
            case Op::BRA:
            case Op::BRN:
            case Op::BSR:
            case Op::BVC:
            case Op::BVS:
            case Op::CLR:
            case Op::CMP8:
            case Op::CMP16:
            case Op::COM:
            case Op::CWAI:
            case Op::DAA:
            case Op::DEC:
            case Op::EOR:
            case Op::EXG:
            case Op::INC:
            case Op::JMP:
            case Op::JSR:
            case Op::LD8:
            case Op::LD16:
            case Op::LEA:
            case Op::LSL:
            case Op::LSR:
            case Op::MUL:
            case Op::NEG:
            case Op::NOP:
            case Op::OR:
            case Op::ORCC:
            case Op::PSH:
            case Op::PUL:
            case Op::ROL:
            case Op::ROR:
            case Op::RTI:
            case Op::RTS:
            case Op::SBC:
            case Op::SEX:
            case Op::ST8:
            case Op::ST16:
            case Op::SUB8:
            case Op::SUB16:
            case Op::SWI:
            case Op::SWI2:
            case Op::SWI3:
            case Op::SYNC:
            case Op::TFR:
            case Op::TST:
            case Op::FIRQ:
            case Op::IRQ:
            case Op::NMI:
            case Op::RESTART:
                break;
        }
        
        // Store result
        
        prevOp = opcode->op;
    }
    
    return true;
}
