#include "instructions.h"
#include <stdio.h>

Instruction instructions[256] = {
    {"NOP ", 1, 4},         {"LD BC, d16", 3, 12},   {"LD BC, A", 1, 8},
    {"INC BC", 1, 8},       {"INC B", 1, 4},         {"DEC B", 1, 4},
    {"LD B, d8", 2, 8},     {"RLCA ", 1, 4},         {"LD a16, SP", 3, 20},
    {"ADD HL, BC", 1, 8},   {"LD A, BC", 1, 8},      {"DEC BC", 1, 8},
    {"INC C", 1, 4},        {"DEC C", 1, 4},         {"LD C, d8", 2, 8},
    {"RRCA ", 1, 4},        {"STOP d8", 2, 4},       {"LD DE, d16", 3, 12},
    {"LD DE, A", 1, 8},     {"INC DE", 1, 8},        {"INC D", 1, 4},
    {"DEC D", 1, 4},        {"LD D, d8", 2, 8},      {"RLA ", 1, 4},
    {"JR r8", 2, 12},       {"ADD HL, DE", 1, 8},    {"LD A, DE", 1, 8},
    {"DEC DE", 1, 8},       {"INC E", 1, 4},         {"DEC E", 1, 4},
    {"LD E, d8", 2, 8},     {"RRA ", 1, 4},          {"JR NZ, r8", 2, 8},
    {"LD HL, d16", 3, 12},  {"LD HL, A", 1, 8},      {"INC HL", 1, 8},
    {"INC H", 1, 4},        {"DEC H", 1, 4},         {"LD H, d8", 2, 8},
    {"DAA ", 1, 4},         {"JR Z, r8", 2, 8},      {"ADD HL, HL", 1, 8},
    {"LD A, HL", 1, 8},     {"DEC HL", 1, 8},        {"INC L", 1, 4},
    {"DEC L", 1, 4},        {"LD L, d8", 2, 8},      {"CPL ", 1, 4},
    {"JR NC, r8", 2, 8},    {"LD SP, d16", 3, 12},   {"LD HL, A", 1, 8},
    {"INC SP", 1, 8},       {"INC HL", 1, 12},       {"DEC HL", 1, 12},
    {"LD HL, d8", 2, 12},   {"SCF ", 1, 4},          {"JR C, r8", 2, 8},
    {"ADD HL, SP", 1, 8},   {"LD A, HL", 1, 8},      {"DEC SP", 1, 8},
    {"INC A", 1, 4},        {"DEC A", 1, 4},         {"LD A, d8", 2, 8},
    {"CCF ", 1, 4},         {"LD B, B", 1, 4},       {"LD B, C", 1, 4},
    {"LD B, D", 1, 4},      {"LD B, E", 1, 4},       {"LD B, H", 1, 4},
    {"LD B, L", 1, 4},      {"LD B, HL", 1, 8},      {"LD B, A", 1, 4},
    {"LD C, B", 1, 4},      {"LD C, C", 1, 4},       {"LD C, D", 1, 4},
    {"LD C, E", 1, 4},      {"LD C, H", 1, 4},       {"LD C, L", 1, 4},
    {"LD C, HL", 1, 8},     {"LD C, A", 1, 4},       {"LD D, B", 1, 4},
    {"LD D, C", 1, 4},      {"LD D, D", 1, 4},       {"LD D, E", 1, 4},
    {"LD D, H", 1, 4},      {"LD D, L", 1, 4},       {"LD D, HL", 1, 8},
    {"LD D, A", 1, 4},      {"LD E, B", 1, 4},       {"LD E, C", 1, 4},
    {"LD E, D", 1, 4},      {"LD E, E", 1, 4},       {"LD E, H", 1, 4},
    {"LD E, L", 1, 4},      {"LD E, HL", 1, 8},      {"LD E, A", 1, 4},
    {"LD H, B", 1, 4},      {"LD H, C", 1, 4},       {"LD H, D", 1, 4},
    {"LD H, E", 1, 4},      {"LD H, H", 1, 4},       {"LD H, L", 1, 4},
    {"LD H, HL", 1, 8},     {"LD H, A", 1, 4},       {"LD L, B", 1, 4},
    {"LD L, C", 1, 4},      {"LD L, D", 1, 4},       {"LD L, E", 1, 4},
    {"LD L, H", 1, 4},      {"LD L, L", 1, 4},       {"LD L, HL", 1, 8},
    {"LD L, A", 1, 4},      {"LD HL, B", 1, 8},      {"LD HL, C", 1, 8},
    {"LD HL, D", 1, 8},     {"LD HL, E", 1, 8},      {"LD HL, H", 1, 8},
    {"LD HL, L", 1, 8},     {"HALT ", 1, 4},         {"LD HL, A", 1, 8},
    {"LD A, B", 1, 4},      {"LD A, C", 1, 4},       {"LD A, D", 1, 4},
    {"LD A, E", 1, 4},      {"LD A, H", 1, 4},       {"LD A, L", 1, 4},
    {"LD A, HL", 1, 8},     {"LD A, A", 1, 4},       {"ADD A, B", 1, 4},
    {"ADD A, C", 1, 4},     {"ADD A, D", 1, 4},      {"ADD A, E", 1, 4},
    {"ADD A, H", 1, 4},     {"ADD A, L", 1, 4},      {"ADD A, HL", 1, 8},
    {"ADD A, A", 1, 4},     {"ADC A, B", 1, 4},      {"ADC A, C", 1, 4},
    {"ADC A, D", 1, 4},     {"ADC A, E", 1, 4},      {"ADC A, H", 1, 4},
    {"ADC A, L", 1, 4},     {"ADC A, HL", 1, 8},     {"ADC A, A", 1, 4},
    {"SUB B", 1, 4},        {"SUB C", 1, 4},         {"SUB D", 1, 4},
    {"SUB E", 1, 4},        {"SUB H", 1, 4},         {"SUB L", 1, 4},
    {"SUB HL", 1, 8},       {"SUB A", 1, 4},         {"SBC A, B", 1, 4},
    {"SBC A, C", 1, 4},     {"SBC A, D", 1, 4},      {"SBC A, E", 1, 4},
    {"SBC A, H", 1, 4},     {"SBC A, L", 1, 4},      {"SBC A, HL", 1, 8},
    {"SBC A, A", 1, 4},     {"AND B", 1, 4},         {"AND C", 1, 4},
    {"AND D", 1, 4},        {"AND E", 1, 4},         {"AND H", 1, 4},
    {"AND L", 1, 4},        {"AND HL", 1, 8},        {"AND A", 1, 4},
    {"XOR B", 1, 4},        {"XOR C", 1, 4},         {"XOR D", 1, 4},
    {"XOR E", 1, 4},        {"XOR H", 1, 4},         {"XOR L", 1, 4},
    {"XOR HL", 1, 8},       {"XOR A", 1, 4},         {"OR B", 1, 4},
    {"OR C", 1, 4},         {"OR D", 1, 4},          {"OR E", 1, 4},
    {"OR H", 1, 4},         {"OR L", 1, 4},          {"OR HL", 1, 8},
    {"OR A", 1, 4},         {"CP B", 1, 4},          {"CP C", 1, 4},
    {"CP D", 1, 4},         {"CP E", 1, 4},          {"CP H", 1, 4},
    {"CP L", 1, 4},         {"CP HL", 1, 8},         {"CP A", 1, 4},
    {"RET NZ", 1, 8},       {"POP BC", 1, 12},       {"JP NZ, a16", 3, 12},
    {"JP a16", 3, 16},      {"CALL NZ, a16", 3, 12}, {"PUSH BC", 1, 16},
    {"ADD A, d8", 2, 8},    {"RST 00H", 1, 16},      {"RET Z", 1, 8},
    {"RET ", 1, 16},        {"JP Z, a16", 3, 12},    {"PREFIX ", 1, 4},
    {"CALL Z, a16", 3, 12}, {"CALL a16", 3, 24},     {"ADC A, d8", 2, 8},
    {"RST 08H", 1, 16},     {"RET NC", 1, 8},        {"POP DE", 1, 12},
    {"JP NC, a16", 3, 12},  {"ILLEGAL_D3 ", 1, 4},   {"CALL NC, a16", 3, 12},
    {"PUSH DE", 1, 16},     {"SUB d8", 2, 8},        {"RST 10H", 1, 16},
    {"RET C", 1, 8},        {"RETI ", 1, 16},        {"JP C, a16", 3, 12},
    {"ILLEGAL_DB ", 1, 4},  {"CALL C, a16", 3, 12},  {"ILLEGAL_DD ", 1, 4},
    {"SBC A, d8", 2, 8},    {"RST 18H", 1, 16},      {"LDH a8, A", 2, 12},
    {"POP HL", 1, 12},      {"LD C, A", 1, 8},       {"ILLEGAL_E3 ", 1, 4},
    {"ILLEGAL_E4 ", 1, 4},  {"PUSH HL", 1, 16},      {"AND d8", 2, 8},
    {"RST 20H", 1, 16},     {"ADD SP, r8", 2, 16},   {"JP HL", 1, 4},
    {"LD a16, A", 3, 16},   {"ILLEGAL_EB ", 1, 4},   {"ILLEGAL_EC ", 1, 4},
    {"ILLEGAL_ED ", 1, 4},  {"XOR d8", 2, 8},        {"RST 28H", 1, 16},
    {"LDH A, a8", 2, 12},   {"POP AF", 1, 12},       {"LD A, C", 1, 8},
    {"DI ", 1, 4},          {"ILLEGAL_F4 ", 1, 4},   {"PUSH AF", 1, 16},
    {"OR d8", 2, 8},        {"RST 30H", 1, 16},      {"LD HL, SP, r8", 2, 12},
    {"LD SP, HL", 1, 8},    {"LD A, a16", 3, 16},    {"EI ", 1, 4},
    {"ILLEGAL_FC ", 1, 4},  {"ILLEGAL_FD ", 1, 4},   {"CP d8", 2, 8},
    {"RST 38H", 1, 16}};

Instruction prefixed[256] = {
    {"RLC B", 2, 8},      {"RLC C", 2, 8},      {"RLC D", 2, 8},
    {"RLC E", 2, 8},      {"RLC H", 2, 8},      {"RLC L", 2, 8},
    {"RLC HL", 2, 16},    {"RLC A", 2, 8},      {"RRC B", 2, 8},
    {"RRC C", 2, 8},      {"RRC D", 2, 8},      {"RRC E", 2, 8},
    {"RRC H", 2, 8},      {"RRC L", 2, 8},      {"RRC HL", 2, 16},
    {"RRC A", 2, 8},      {"RL B", 2, 8},       {"RL C", 2, 8},
    {"RL D", 2, 8},       {"RL E", 2, 8},       {"RL H", 2, 8},
    {"RL L", 2, 8},       {"RL HL", 2, 16},     {"RL A", 2, 8},
    {"RR B", 2, 8},       {"RR C", 2, 8},       {"RR D", 2, 8},
    {"RR E", 2, 8},       {"RR H", 2, 8},       {"RR L", 2, 8},
    {"RR HL", 2, 16},     {"RR A", 2, 8},       {"SLA B", 2, 8},
    {"SLA C", 2, 8},      {"SLA D", 2, 8},      {"SLA E", 2, 8},
    {"SLA H", 2, 8},      {"SLA L", 2, 8},      {"SLA HL", 2, 16},
    {"SLA A", 2, 8},      {"SRA B", 2, 8},      {"SRA C", 2, 8},
    {"SRA D", 2, 8},      {"SRA E", 2, 8},      {"SRA H", 2, 8},
    {"SRA L", 2, 8},      {"SRA HL", 2, 16},    {"SRA A", 2, 8},
    {"SWAP B", 2, 8},     {"SWAP C", 2, 8},     {"SWAP D", 2, 8},
    {"SWAP E", 2, 8},     {"SWAP H", 2, 8},     {"SWAP L", 2, 8},
    {"SWAP HL", 2, 16},   {"SWAP A", 2, 8},     {"SRL B", 2, 8},
    {"SRL C", 2, 8},      {"SRL D", 2, 8},      {"SRL E", 2, 8},
    {"SRL H", 2, 8},      {"SRL L", 2, 8},      {"SRL HL", 2, 16},
    {"SRL A", 2, 8},      {"BIT 0, B", 2, 8},   {"BIT 0, C", 2, 8},
    {"BIT 0, D", 2, 8},   {"BIT 0, E", 2, 8},   {"BIT 0, H", 2, 8},
    {"BIT 0, L", 2, 8},   {"BIT 0, HL", 2, 12}, {"BIT 0, A", 2, 8},
    {"BIT 1, B", 2, 8},   {"BIT 1, C", 2, 8},   {"BIT 1, D", 2, 8},
    {"BIT 1, E", 2, 8},   {"BIT 1, H", 2, 8},   {"BIT 1, L", 2, 8},
    {"BIT 1, HL", 2, 12}, {"BIT 1, A", 2, 8},   {"BIT 2, B", 2, 8},
    {"BIT 2, C", 2, 8},   {"BIT 2, D", 2, 8},   {"BIT 2, E", 2, 8},
    {"BIT 2, H", 2, 8},   {"BIT 2, L", 2, 8},   {"BIT 2, HL", 2, 12},
    {"BIT 2, A", 2, 8},   {"BIT 3, B", 2, 8},   {"BIT 3, C", 2, 8},
    {"BIT 3, D", 2, 8},   {"BIT 3, E", 2, 8},   {"BIT 3, H", 2, 8},
    {"BIT 3, L", 2, 8},   {"BIT 3, HL", 2, 12}, {"BIT 3, A", 2, 8},
    {"BIT 4, B", 2, 8},   {"BIT 4, C", 2, 8},   {"BIT 4, D", 2, 8},
    {"BIT 4, E", 2, 8},   {"BIT 4, H", 2, 8},   {"BIT 4, L", 2, 8},
    {"BIT 4, HL", 2, 12}, {"BIT 4, A", 2, 8},   {"BIT 5, B", 2, 8},
    {"BIT 5, C", 2, 8},   {"BIT 5, D", 2, 8},   {"BIT 5, E", 2, 8},
    {"BIT 5, H", 2, 8},   {"BIT 5, L", 2, 8},   {"BIT 5, HL", 2, 12},
    {"BIT 5, A", 2, 8},   {"BIT 6, B", 2, 8},   {"BIT 6, C", 2, 8},
    {"BIT 6, D", 2, 8},   {"BIT 6, E", 2, 8},   {"BIT 6, H", 2, 8},
    {"BIT 6, L", 2, 8},   {"BIT 6, HL", 2, 12}, {"BIT 6, A", 2, 8},
    {"BIT 7, B", 2, 8},   {"BIT 7, C", 2, 8},   {"BIT 7, D", 2, 8},
    {"BIT 7, E", 2, 8},   {"BIT 7, H", 2, 8},   {"BIT 7, L", 2, 8},
    {"BIT 7, HL", 2, 12}, {"BIT 7, A", 2, 8},   {"RES 0, B", 2, 8},
    {"RES 0, C", 2, 8},   {"RES 0, D", 2, 8},   {"RES 0, E", 2, 8},
    {"RES 0, H", 2, 8},   {"RES 0, L", 2, 8},   {"RES 0, HL", 2, 16},
    {"RES 0, A", 2, 8},   {"RES 1, B", 2, 8},   {"RES 1, C", 2, 8},
    {"RES 1, D", 2, 8},   {"RES 1, E", 2, 8},   {"RES 1, H", 2, 8},
    {"RES 1, L", 2, 8},   {"RES 1, HL", 2, 16}, {"RES 1, A", 2, 8},
    {"RES 2, B", 2, 8},   {"RES 2, C", 2, 8},   {"RES 2, D", 2, 8},
    {"RES 2, E", 2, 8},   {"RES 2, H", 2, 8},   {"RES 2, L", 2, 8},
    {"RES 2, HL", 2, 16}, {"RES 2, A", 2, 8},   {"RES 3, B", 2, 8},
    {"RES 3, C", 2, 8},   {"RES 3, D", 2, 8},   {"RES 3, E", 2, 8},
    {"RES 3, H", 2, 8},   {"RES 3, L", 2, 8},   {"RES 3, HL", 2, 16},
    {"RES 3, A", 2, 8},   {"RES 4, B", 2, 8},   {"RES 4, C", 2, 8},
    {"RES 4, D", 2, 8},   {"RES 4, E", 2, 8},   {"RES 4, H", 2, 8},
    {"RES 4, L", 2, 8},   {"RES 4, HL", 2, 16}, {"RES 4, A", 2, 8},
    {"RES 5, B", 2, 8},   {"RES 5, C", 2, 8},   {"RES 5, D", 2, 8},
    {"RES 5, E", 2, 8},   {"RES 5, H", 2, 8},   {"RES 5, L", 2, 8},
    {"RES 5, HL", 2, 16}, {"RES 5, A", 2, 8},   {"RES 6, B", 2, 8},
    {"RES 6, C", 2, 8},   {"RES 6, D", 2, 8},   {"RES 6, E", 2, 8},
    {"RES 6, H", 2, 8},   {"RES 6, L", 2, 8},   {"RES 6, HL", 2, 16},
    {"RES 6, A", 2, 8},   {"RES 7, B", 2, 8},   {"RES 7, C", 2, 8},
    {"RES 7, D", 2, 8},   {"RES 7, E", 2, 8},   {"RES 7, H", 2, 8},
    {"RES 7, L", 2, 8},   {"RES 7, HL", 2, 16}, {"RES 7, A", 2, 8},
    {"SET 0, B", 2, 8},   {"SET 0, C", 2, 8},   {"SET 0, D", 2, 8},
    {"SET 0, E", 2, 8},   {"SET 0, H", 2, 8},   {"SET 0, L", 2, 8},
    {"SET 0, HL", 2, 16}, {"SET 0, A", 2, 8},   {"SET 1, B", 2, 8},
    {"SET 1, C", 2, 8},   {"SET 1, D", 2, 8},   {"SET 1, E", 2, 8},
    {"SET 1, H", 2, 8},   {"SET 1, L", 2, 8},   {"SET 1, HL", 2, 16},
    {"SET 1, A", 2, 8},   {"SET 2, B", 2, 8},   {"SET 2, C", 2, 8},
    {"SET 2, D", 2, 8},   {"SET 2, E", 2, 8},   {"SET 2, H", 2, 8},
    {"SET 2, L", 2, 8},   {"SET 2, HL", 2, 16}, {"SET 2, A", 2, 8},
    {"SET 3, B", 2, 8},   {"SET 3, C", 2, 8},   {"SET 3, D", 2, 8},
    {"SET 3, E", 2, 8},   {"SET 3, H", 2, 8},   {"SET 3, L", 2, 8},
    {"SET 3, HL", 2, 16}, {"SET 3, A", 2, 8},   {"SET 4, B", 2, 8},
    {"SET 4, C", 2, 8},   {"SET 4, D", 2, 8},   {"SET 4, E", 2, 8},
    {"SET 4, H", 2, 8},   {"SET 4, L", 2, 8},   {"SET 4, HL", 2, 16},
    {"SET 4, A", 2, 8},   {"SET 5, B", 2, 8},   {"SET 5, C", 2, 8},
    {"SET 5, D", 2, 8},   {"SET 5, E", 2, 8},   {"SET 5, H", 2, 8},
    {"SET 5, L", 2, 8},   {"SET 5, HL", 2, 16}, {"SET 5, A", 2, 8},
    {"SET 6, B", 2, 8},   {"SET 6, C", 2, 8},   {"SET 6, D", 2, 8},
    {"SET 6, E", 2, 8},   {"SET 6, H", 2, 8},   {"SET 6, L", 2, 8},
    {"SET 6, HL", 2, 16}, {"SET 6, A", 2, 8},   {"SET 7, B", 2, 8},
    {"SET 7, C", 2, 8},   {"SET 7, D", 2, 8},   {"SET 7, E", 2, 8},
    {"SET 7, H", 2, 8},   {"SET 7, L", 2, 8},   {"SET 7, HL", 2, 16},
    {"SET 7, A", 2, 8}};
