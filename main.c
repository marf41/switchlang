#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// TODO: types, strings, wait, stop, multiple parsers

#ifndef MAX_PROG
#define MAX_PROG 255
#endif
#ifndef MAX_USER
#define MAX_USER 16
#endif
#ifndef MAX_DATA
#define MAX_DATA 128
#endif

#ifndef stack_t
#define stack_t int
#endif

#define INCLUDE_WORD_TESTS 1

#ifdef INCLUDE_WORD_TESTS
#define WORD_TEST(a, b) false, a, b
#define WORD_HIDE true
#else
#define WORD_TEST(a, b)
#define WORD_HIDE
#endif

#include "enums.h"
#include "structs.h"

#define HASH2(a, b) ((a << 8) | b)
#define HASH3(a, b, c) ((a << 16) | (b << 8) | c)
#define HASH4(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

#define STACK_MAX ((stack_t)((1ULL << (sizeof(stack_t) * 8 - 1)) - 1))
#define STACK_MIN ((stack_t)(-(1ULL << (sizeof(stack_t) * 8 - 1))))

void parser_exec(struct parser_t *parser);
void (*word_table[])(struct parser_t *);
struct word_t find_word(enum parser_word_t word);
struct word_t *words[];

#define PROG_PC parser->prog[parser->pc].op

bool is_op(struct cell_t cell, uint8_t i)
{
    switch (i)
    {
    case 0:
        return cell.is_op0;
    case 1:
        return cell.is_op1;
    case 2:
        return cell.is_op2;
    case 3:
        return cell.is_op3;
    default:
        return false;
    }
}
void set_op(struct cell_t *cell, uint8_t i, uint8_t v)
{
    switch (i)
    {
    case 0:
        cell->is_op0 = v;
        break;
    case 1:
        cell->is_op1 = v;
        break;
    case 2:
        cell->is_op2 = v;
        break;
    case 3:
        cell->is_op3 = v;
        break;
    }
}
// #define add_word(parser, word) parser->prog[parser->pc++] = word
// #define add_word(parser, word) \
    // parser->prog[parser->pc++] = (struct cell_t) { .is_op = {1, 0, 0, 0}, .op = word }
uint8_t add_word(struct parser_t *parser, enum parser_word_t word)
{
    uint8_t pc = parser->pc;
    enum word_place_t place = find_word(word).place;
    if (parser->prog[parser->pc].op == 0)
    {
        parser->prog[parser->pc].op = word;
        parser->prog[parser->pc].place = place;
        if (place > ANY)
        {
            parser->pc++;
            parser->sub = 0;
        }
        return pc;
    }
    if (place != SOLO && parser->prog[parser->pc].place == ANY)
    {
        if (parser->sub < 3)
        {
            set_op(&parser->prog[parser->pc], parser->sub, 1);
            parser->prog[parser->pc].v.op[parser->sub] = word;
            parser->sub++;
            if (place == LAST)
            {
                parser->sub += 5;
            }
            return pc;
        }
    }
    parser->pc++;
    parser->sub = 0;
    parser->prog[parser->pc].op = word;
    parser->prog[parser->pc].place = place;
    pc = parser->pc;
    if (place > ANY)
    {
        parser->pc++;
    }
    return pc;
}
#define DBG(level, fmt, ...)            \
    do                                  \
    {                                   \
        if (parser->debug >= level)     \
        {                               \
            printf(fmt, ##__VA_ARGS__); \
        }                               \
    } while (0)

#define ADD_SKIP(offset) parser->skip[parser->skc++] = offset
#define OP_WORD(name, op) \
    void word_##name(struct parser_t *parser) { push(parser, pop(parser) op pop(parser)); }
#define OP_SWAP_WORD(name, op)                    \
    void word_##name(struct parser_t *parser)     \
    {                                             \
        swap(parser);                             \
        push(parser, pop(parser) op pop(parser)); \
    }
#define GET_DATA(addr, i) parser->prog[addr].len
#define GET_UDATA(addr) parser->prog[addr].v.data.value.u
#define PC_DATA(i) GET_DATA(parser->pc, i)
#define PC_UDATA() GET_UDATA(parser->pc)
#define SET_DATA(addr, i, val)    \
    parser->prog[addr].len = val; \
    DBG(2, "Setting data at %d.%d to %d\n", addr, i, val)
#define SET_UDATA(addr, val) parser->prog[addr].v.data.value.u = val
#define SET_LEN(addr, len) parser->prog[addr].len = len
#define GET_LEN(addr) parser->prog[addr].len
// #define SAVE_SKIP() SET_DATA(parser->skip[parser->skc - 1], 0, parser->pc)
void save_skip(struct parser_t *parser)
{
    if (parser->skc > 0)
    {
        if (parser->prog[parser->pc].op != 0)
        {
            parser->pc++;
            parser->sub = 0;
        }
        SET_DATA(parser->skip[parser->skc - 1], 0, parser->pc);
    }
}
#define CALL_WORD(w)                                                              \
    if (w < WORD_END_OF_LIST_OVER)                                                \
    {                                                                             \
        word_table[w](parser);                                                    \
    }                                                                             \
    else                                                                          \
    {                                                                             \
        printf("Error: invalid word %d @ %d (%d)\n", w, parser->pc, parser->sub); \
    }

void parser_init(struct parser_t *parser)
{
    // Counters
    parser->pc = 0;    // program
    parser->sub = 0;   // subprogram
    parser->sc = 0;    // stack
    parser->skc = 0;   // skip
    parser->rc = 0;    // return
    parser->uc = 0;    // user
    parser->debug = 0; // debug level

    parser->mode = NONE;
    for (uint8_t i = 0; i < MAX_PROG; i++)
    {
        parser->prog[i] = (struct cell_t){0};
    }
    for (uint8_t i = 0; i < MAX_DATA; i++)
    {
        parser->data[i] = (struct data_t){0};
    }
}

// #define push(parser, value) parser->stack[parser->sc++] = (value)

void push(struct parser_t *parser, stack_t value)
{
    parser->stack[parser->sc++] = value;
}

// #define pop(parser) parser->stack[--parser->sc]
// #define pop(parser) (parser->sc > 0 ? parser->stack[--parser->sc] : (printf("\nError: stack is empty [%d]\n", parser->pc), -1))

stack_t pop(struct parser_t *parser)
{
    if (parser->sc == 0)
    {
        printf("Error: stack is empty\n");
        return -1;
    }
    return parser->stack[--parser->sc];
}

// offset = 1 for top of stack
stack_t peek(struct parser_t *parser, int offset)
{
    if (parser->sc < offset)
    {
        printf("Error: stack isn't big enough [%d]\n", parser->pc);
        return -1;
    }
    return parser->stack[parser->sc - offset];
}

void swap(struct parser_t *parser)
{
    stack_t a = pop(parser);
    stack_t b = pop(parser);
    push(parser, a);
    push(parser, b);
}

void set_pc(struct parser_t *parser, uint8_t pc)
{
    if ((pc == 0) || (pc > parser->progsize))
    {
        printf("Error: PC jump %d -> %d out of bounds (length %d)\n", parser->pc, pc, parser->progsize);
        return;
    }
    DBG(2, "Setting PC to %d\n", pc);
    parser->pc = pc - 1;
    parser->sub = 99;
}

void word_dot(struct parser_t *parser)
{
    printf("%d ", pop(parser));
}

void word_print_stack(struct parser_t *parser)
{
    printf("<%d> ", parser->sc);
    for (int i = 0; i < parser->sc; i++)
    {
        printf("%d ", parser->stack[i]);
    }
    printf("\n");
}

OP_WORD(add, +);
OP_SWAP_WORD(subtract, -);
OP_WORD(multiply, *);
OP_SWAP_WORD(divide, /);
OP_WORD(equal, ==);
OP_WORD(not_equal, !=);
OP_SWAP_WORD(greater, >);
OP_SWAP_WORD(less, <);
OP_SWAP_WORD(greater_equal, >=);
OP_SWAP_WORD(less_equal, <=);
OP_WORD(xor, ^);
OP_SWAP_WORD(shl, <<);
OP_SWAP_WORD(shr, >>);
OP_SWAP_WORD(mod, %);

void word_and(struct parser_t *parser)
{
    stack_t a = pop(parser);
    stack_t b = pop(parser);
    push(parser, a & b);
}

void word_or(struct parser_t *parser)
{
    stack_t a = pop(parser);
    stack_t b = pop(parser);
    push(parser, a | b);
}

void word_abs(struct parser_t *parser)
{
    stack_t temp = pop(parser);
    if (temp < 0)
    {
        temp = -temp;
    }
    push(parser, temp);
}

void word_not(struct parser_t *parser)
{
    if (parser->sc > 0)
    {
        parser->stack[parser->sc - 1] = !parser->stack[parser->sc - 1];
    }
}

void word_dup(struct parser_t *parser)
{
    stack_t temp = peek(parser, 1);
    push(parser, temp);
}

void word_drop(struct parser_t *parser)
{
    if (parser->sc > 0)
    {
        parser->sc--;
    }
}

void word_swap(struct parser_t *parser)
{
    swap(parser);
}

void word_over(struct parser_t *parser)
{
    stack_t temp = peek(parser, 2);
    push(parser, temp);
}

void word_rot(struct parser_t *parser)
{
    if (parser->sc < 3)
    {
        printf("Error: not enough items on stack for rot [%d]\n", parser->pc);
        return;
    }
    stack_t temp = parser->stack[parser->sc - 3];
    parser->stack[parser->sc - 3] = parser->stack[parser->sc - 2];
    parser->stack[parser->sc - 2] = parser->stack[parser->sc - 1];
    parser->stack[parser->sc - 1] = temp;
}

void word_rot_back(struct parser_t *parser)
{
    if (parser->sc < 3)
    {
        printf("Error: not enough items on stack for rot_back [%d]\n", parser->pc);
        return;
    }
    stack_t temp = parser->stack[parser->sc - 1];
    parser->stack[parser->sc - 1] = parser->stack[parser->sc - 2];
    parser->stack[parser->sc - 2] = parser->stack[parser->sc - 3];
    parser->stack[parser->sc - 3] = temp;
}

void word_nip(struct parser_t *parser)
{
    if (parser->sc < 2)
    {
        printf("Error: not enough items on stack for nip [%d]\n", parser->pc);
        return;
    }
    stack_t temp = pop(parser);
    parser->stack[parser->sc - 1] = temp;
}

void word_tuck(struct parser_t *parser)
{
    stack_t temp = peek(parser, 1);
    swap(parser);
    push(parser, temp);
}

void word_nop(struct parser_t *parser) {}

void word_literal(struct parser_t *parser)
{
    DBG(2, "Literal: %d @ %d\n", PC_UDATA(), parser->pc);
    push(parser, PC_UDATA());
}

void word_set(struct parser_t *parser)
{
    stack_t addr = pop(parser);
    stack_t value = pop(parser);
    if (addr >= MAX_DATA)
    {
        printf("Error: set address %d out of bounds\n", addr);
        return;
    }
    parser->data[addr].value.u = value;
}

void word_get(struct parser_t *parser)
{
    stack_t addr = pop(parser);
    if (addr >= MAX_DATA)
    {
        printf("Error: get address %d out of bounds\n", addr);
        return;
    }
    push(parser, parser->data[addr].value.u);
}

void word_memory_min(struct parser_t *parser)
{
    stack_t addr = pop(parser);
    stack_t value = pop(parser);
    if (addr >= MAX_DATA)
    {
        printf("Error: set address %d out of bounds\n", addr);
        return;
    }
    if (parser->data[addr].value.u > value)
    {
        parser->data[addr].value.u = value;
    }
}

void word_memory_max(struct parser_t *parser)
{
    stack_t addr = pop(parser);
    stack_t value = pop(parser);
    if (addr >= MAX_DATA)
    {
        printf("Error: set address %d out of bounds\n", addr);
        return;
    }
    if (parser->data[addr].value.u < value)
    {
        parser->data[addr].value.u = value;
    }
}

void word_clock(struct parser_t *parser)
{
    push(parser, (stack_t)clock());
}

void word_increment(struct parser_t *parser)
{
    parser->stack[parser->sc - 1]++;
}

void word_decrement(struct parser_t *parser)
{
    parser->stack[parser->sc - 1]--;
}

void word_string(struct parser_t *parser)
{
    for (uint8_t i = 0; i < parser->prog[parser->pc].len; i++)
    {
        printf("%c", parser->prog[parser->pc].v.op[i]);
    }
}

void word_emit(struct parser_t *parser)
{
    printf("%c", (char)(pop(parser) & 0x7F));
}

void word_newline(struct parser_t *parser)
{
    printf("\n");
}

void word_if(struct parser_t *parser)
{
    stack_t temp = pop(parser);
    DBG(3, "IF %d\n", temp);
    uint8_t addr = PC_DATA(0);
    if (temp <= 0)
    {
        set_pc(parser, addr);
        DBG(3, "IF skipping to %d\n", parser->pc);
    }
    else
    {
        DBG(3, "IF ends at: %d\n", addr);
        // parser->skip = parser->prog[parser->pc + 1] - 1;
    }
}

void word_if_else(struct parser_t *parser)
{
    set_pc(parser, PC_DATA(parser->sub));
}

void word_loop_end(struct parser_t *parser)
{
    stack_t temp = pop(parser);
    if (temp > 0)
    {
        uint8_t addr = PC_DATA(parser->sub);
        set_pc(parser, addr);
    }
}

void word_define(struct parser_t *parser)
{
    set_pc(parser, PC_DATA(parser->sub));
}

void word_define_end(struct parser_t *parser)
{
    set_pc(parser, parser->ret[--parser->rc]);
}

void word_return(struct parser_t *parser)
{
    set_pc(parser, parser->ret[--parser->rc]);
}

void word_call(struct parser_t *parser)
{
    if (parser->rc >= 16)
    {
        printf("\nError: return stack overflow\n");
        return;
    }
    parser->ret[parser->rc++] = parser->pc + 1;
    uint8_t addr = PC_DATA(parser->sub);
    set_pc(parser, addr);
}

void word_execute(struct parser_t *parser)
{
    if (parser->rc >= 16)
    {
        printf("\nError: return stack overflow\n");
        return;
    }
    parser->ret[parser->rc++] = parser->pc + 1;
    uint8_t addr = pop(parser);
    set_pc(parser, addr);
}

void word_goto(struct parser_t *parser)
{
    set_pc(parser, pop(parser));
}

#pragma region parse word definitions

void parse_if(struct parser_t *parser)
{
    add_word(parser, WORD_IF);
    ADD_SKIP(parser->pc - 1);
}

void parse_if_else(struct parser_t *parser)
{
    add_word(parser, WORD_IF_ELSE);
    if (parser->skc > 0)
    {
        save_skip(parser);
        parser->skip[parser->skc - 1] = parser->pc - 1;
    }
}

void parse_if_end(struct parser_t *parser)
{

    save_skip(parser);
    parser->skc--;
}

void parse_loop(struct parser_t *parser)
{
    ADD_SKIP(parser->pc);
}

void parse_loop_end(struct parser_t *parser)
{
    uint8_t pc = add_word(parser, WORD_LOOP_END);
    if (parser->skc > 0)
    {
        // add_word(parser, parser->skip[parser->skc - 1]);
        SET_DATA(pc, parser->sub - 6, parser->skip[parser->skc - 1]);
    }
    parser->skc--;
}

void parse_define(struct parser_t *parser)
{
    add_word(parser, WORD_DEFINE);
    ADD_SKIP(parser->pc - 1);
    parser->uword[parser->uc++].id = parser->pc;
    parser->mode = HASH;
}

void parse_define_end(struct parser_t *parser)
{
    add_word(parser, WORD_DEFINE_END);
    save_skip(parser);
    parser->skc--;
}

void parse_pc(struct parser_t *parser)
{
    printf("PC%d.%d ", parser->pc, parser->sub);
}

void parse_solo(struct parser_t *parser)
{
    parser->pc++;
    parser->sub = 0;
}

void parse_debug(struct parser_t *parser)
{
    parser->debug++;
    printf("Debug level: %d\n", parser->debug);
}

void parse_words_list(struct parser_t *parser)
{
    for (int i = 1; i <= 4; i++)
    {
        int id = 0;
        while (words[i][id].info.id != -1)
        {
            printf("%s ", words[i][id].name);
            id++;
        }
    }
    printf("\n");
}

void parse_string_end(struct parser_t *parser)
{
    add_word(parser, WORD_STRING_END);
}

void parse_string_begin(struct parser_t *parser)
{
    parser->mode = STRING;
}

#pragma endregion

#ifdef INCLUDE_WORD_TESTS
enum parser_mode_t hash(struct parser_t *parser, size_t len, char *word);
void parse(struct parser_t *parser, char *buf);
void word_test(struct parser_t *parser)
{
    int n = 1;
    int passed = 0;
    int failed = 0;
    int skipped = 0;
    int all = 0;
    int i = 0;
    while (i != WORD_END_OF_LIST_OVER)
    {
        if (word_table[i] == NULL)
        {
            printf("Missing function for word %d\n", i);
            return;
        }
        i++;
    }
    for (i = 1; i <= 4; i++)
    {
        int id = 0;
        while (words[i][id].info.id != -1)
        {
            if (!words[i][id].test_hide)
            {
                all++;
            }
            if (words[i][id].test_before[0] == '\0')
            {
                if (!words[i][id].test_hide)
                {
                    printf("%3d. %s: SKIP\n", n++, words[i][id].name);
                    skipped++;
                }
            }
            else
            {
                printf("%3d. %s '%s' -> '%s'\n", n++, words[i][id].name, words[i][id].test_before, words[i][id].test_after);
                struct parser_t test_parser;
                parser_init(&test_parser);
                parse(&test_parser, words[i][id].test_before);
                parser_exec(&test_parser);
                // hash(&test_parser, words[i][id].len, words[i][id].name);
                char result[255] = {0};
                int rp = 0;
                for (int j = 0; j < test_parser.sc; j++)
                {
                    rp += snprintf(result + rp, 255 - rp, "%d ", test_parser.stack[j]);
                }
                if (rp > 0 && result[rp - 1] == ' ')
                {
                    rp--;
                }
                result[rp] = '\0';
                rp = 0;
                bool pass = true;
                while (result[rp] && words[i][id].test_after[rp] && result[rp] == words[i][id].test_after[rp])
                {
                    rp++;
                }
                if (result[rp] != words[i][id].test_after[rp])
                {
                    pass = false;
                }
                printf("\t'%10s' vs '%10s': %20s\n", result, words[i][id].test_after, pass ? "passed" : "FAILED!");
                if (pass)
                {
                    passed++;
                }
                else
                {
                    failed++;
                }
            }
            id++;
        }
    }
    int ns = all - skipped;
    printf("\nPassed: %d / %d\n", passed, ns);
    printf("Skipped: %d / %d\n", skipped, all);
    if (failed > 0)
    {
        printf("Failed: %d / %d\n", failed, ns);
        if (failed == ns)
        {
            printf("!!! A L L  T E S T S   F A I L E D !!!\n");
        }
        else
        {
            printf("!!! NOT ALL TESTS PASSED! !!!\n");
        }
    }
}
#endif

#pragma region words list

struct word_t words0[] = {
    {1, 0, 0, WORD_LITERAL, "lit", word_literal, SOLO},
    {2, 0, 0, WORD_CALL, "call", word_call, LAST},
    {-1, 0, 0, WORD_END, "NULL", NULL},
};

struct word_t words1[] = {
    {101, 1, '.', WORD_DOT, ".", word_dot, ANY, WORD_TEST("1 2 .", "1")},
    {102, 1, '+', WORD_ADD, "+", word_add, ANY, WORD_TEST("1 2 +", "3")},
    {103, 1, '-', WORD_SUBTRACT, "-", word_subtract, ANY, WORD_TEST("1 2 -", "-1")},
    {104, 1, '*', WORD_MULTIPLY, "*", word_multiply, ANY, WORD_TEST("1 2 *", "2")},
    {105, 1, '/', WORD_DIVIDE, "/", word_divide, ANY, WORD_TEST("4 2 /", "2")},
    {106, 1, ':', WORD_DEFINE, ":", parse_define, SOLO},
    {107, 1, ';', WORD_DEFINE_END, ";", parse_define_end, LAST},
    {108, 1, '=', WORD_EQUAL, "=", word_equal, ANY, WORD_TEST("2 2 =", "1")},
    {109, 1, '>', WORD_GREATER, ">", word_greater, ANY, WORD_TEST("3 2 > 2 2 >", "1 0")},
    {110, 1, '<', WORD_LESS, "<", word_less, ANY, WORD_TEST("2 3 < 2 2 <", "1 0")},
    {111, 1, '"', WORD_STRING_END, "\"", parse_string_end, ANY},
    {-1, 0, 0, WORD_END, "EOF", NULL},
};

struct word_t words2[] = {
    {201, 2, HASH2('.', 's'), WORD_PRINT_STACK, ".s", word_print_stack, ANY},
    {202, 2, HASH2('i', 'f'), WORD_IF, "if", parse_if, SOLO, WORD_TEST("1 if 2 else 4 fi 0 if 3 else 5 fi", "2 5")},
    {203, 2, HASH2('f', 'i'), WORD_IF_END, "fi", parse_if_end, LAST},
    {204, 2, HASH2('<', '='), WORD_LESS_EQUAL, "<=", word_less_equal, ANY, WORD_TEST("2 3 <= 3 2 <=", "1 0")},
    {205, 2, HASH2('>', '='), WORD_GREATER_EQUAL, ">=", word_greater_equal, ANY, WORD_TEST("3 2 >= 2 3 >=", "1 0")},
    {206, 2, HASH2('<', '>'), WORD_NOT_EQUAL, "<>", word_not_equal, ANY, WORD_TEST("2 3 <> 2 2 <>", "1 0")},
    {207, 2, HASH2('o', 'r'), WORD_OR, "or", word_or, ANY, WORD_TEST("0 0 or 0 1 or 1 0 or 1 1 or", "0 1 1 1")},
    {208, 2, HASH2('<', '<'), WORD_SHL, "<<", word_shl, ANY, WORD_TEST("1 2 <<", "4")},
    {209, 2, HASH2('>', '>'), WORD_SHR, ">>", word_shr, ANY, WORD_TEST("4 2 >>", "1")},
    {210, 2, HASH2('1', '+'), WORD_INCREMENT, "1+", word_increment, ANY, WORD_TEST("1 1+", "2")},
    {211, 2, HASH2('1', '-'), WORD_DECREMENT, "1-", word_decrement, ANY, WORD_TEST("2 1-", "1")},
    {212, 2, HASH2('p', 'c'), WORD_PC, "pc", parse_pc, ANY},
    {213, 2, HASH2('.', '"'), WORD_STRING, ".\"", parse_string_begin, SOLO},
    {214, 2, HASH2('c', 'r'), WORD_NEWLINE, "cr", word_newline, ANY},
    {-1, 0, 0, WORD_END, "EOF", NULL},
};

struct word_t words3[] = {
    {301, 3, HASH3('a', 'b', 's'), WORD_ABS, "abs", word_abs, ANY, WORD_TEST("-1 abs", "1")},
    {302, 3, HASH3('d', 'u', 'p'), WORD_DUP, "dup", word_dup, ANY, WORD_TEST("1 dup", "1 1")},
    {303, 3, HASH3('n', 'i', 'p'), WORD_NIP, "nip", word_nip, ANY, WORD_TEST("1 2 nip", "2")},
    {304, 3, HASH3('r', 'o', 't'), WORD_ROT, "rot", word_rot, ANY, WORD_TEST("1 2 3 rot", "2 3 1")},
    {305, 3, HASH3('d', 'b', 'g'), WORD_DEBUG, "dbg", parse_debug},
    {306, 3, HASH3('a', 'n', 'd'), WORD_AND, "and", word_and, ANY, WORD_TEST("0 0 and 0 1 and 1 0 and 1 1 and", "0 0 0 1")},
    {307, 3, HASH3('x', 'o', 'r'), WORD_XOR, "xor", word_xor, ANY, WORD_TEST("0 0 xor 0 1 xor 1 0 xor 1 1 xor", "0 1 1 0")},
    {308, 3, HASH3('n', 'o', 't'), WORD_NOT, "not", word_not, ANY, WORD_TEST("0 not 1 not", "1 0")},
    {309, 3, HASH3('m', 'o', 'd'), WORD_MOD, "mod", word_mod, ANY, WORD_TEST("5 2 mod", "1")},
    {310, 3, HASH3('s', 'e', 't'), WORD_SET, "set", word_set, ANY},
    {311, 3, HASH3('g', 'e', 't'), WORD_GET, "get", word_get, ANY, WORD_TEST("1 99 set 99 get", "1")},
    {312, 3, HASH3('c', 'l', 'k'), WORD_CLOCK, "clk", word_clock, ANY},
    {313, 3, HASH3('r', 'e', 't'), WORD_RETURN, "Ret", word_return, ANY},
    {-1, 0, 0, WORD_END, "EOF", NULL},
};

struct word_t words4[] = {
    {401, 4, HASH4('d', 'r', 'o', 'p'), WORD_DROP, "drop", word_drop, ANY, WORD_TEST("1 2 drop", "1")},
    {402, 4, HASH4('s', 'w', 'a', 'p'), WORD_SWAP, "swap", word_swap, ANY, WORD_TEST("1 2 swap", "2 1")},
    {403, 4, HASH4('o', 'v', 'e', 'r'), WORD_OVER, "over", word_over, ANY, WORD_TEST("1 2 over", "1 2 1")},
    {404, 4, HASH4('t', 'u', 'c', 'k'), WORD_TUCK, "tuck", word_tuck, ANY, WORD_TEST("1 2 tuck", "2 1 2")},
    {405, 4, HASH4('-', 'r', 'o', 't'), WORD_ROT_BACK, "-rot", word_rot_back, ANY, WORD_TEST("1 2 3 -rot", "3 1 2")},
    {406, 4, HASH4('e', 'l', 's', 'e'), WORD_IF_ELSE, "else", parse_if_else, LAST},
    {407, 4, HASH4('l', 'o', 'o', 'p'), WORD_LOOP, "loop", parse_loop, SOLO, WORD_TEST("5 loop 1 swap 1 - dup done drop", "1 1 1 1 1")},
    {408, 4, HASH4('d', 'o', 'n', 'e'), WORD_LOOP_END, "done", parse_loop_end, LAST},
    {409, 4, HASH4('g', 'o', 't', 'o'), WORD_GOTO, "goto", word_goto, ANY, WORD_TEST("4 goto 2 3", "3")},
    {410, 4, HASH4('s', 'o', 'l', 'o'), WORD_SOLO, "solo", parse_solo},
    {411, 4, HASH4('m', 'm', 'i', 'n'), WORD_MEMORY_MIN, "mmin", word_memory_min, ANY},
    {412, 4, HASH4('m', 'm', 'a', 'x'), WORD_MEMORY_MAX, "mmax", word_memory_max, ANY},
    {413, 4, HASH4('w', 'o', 'r', 'd'), WORD_WORDS_LIST, "word", parse_words_list, ANY},
    {414, 4, HASH4('e', 'm', 'i', 't'), WORD_EMIT, "emit", word_emit, ANY},
    {415, 4, HASH4('e', 'x', 'e', 'c'), WORD_EXECUTE, "exec", word_execute, ANY},
#ifdef INCLUDE_WORD_TESTS
    {499, 4, HASH4('t', 'e', 's', 't'), WORD_TEST, "test", word_test, ANY, WORD_HIDE},
#endif
    {-1, 0, 0, WORD_END, "EOF", NULL},
};

#pragma endregion

void (*word_table[])(struct parser_t *) = {
    [WORD_END] = word_nop,
    [WORD_NOP] = word_nop,
    [WORD_DOT] = word_dot,
    [WORD_ADD] = word_add,
    [WORD_SUBTRACT] = word_subtract,
    [WORD_MULTIPLY] = word_multiply,
    [WORD_DIVIDE] = word_divide,
    [WORD_PRINT_STACK] = word_print_stack,
    [WORD_IF] = word_if,
    [WORD_IF_ELSE] = word_if_else,
    [WORD_IF_END] = word_nop,
    [WORD_ABS] = word_abs,
    [WORD_DUP] = word_dup,
    [WORD_NIP] = word_nip,
    [WORD_ROT] = word_rot,
    [WORD_DROP] = word_drop,
    [WORD_SWAP] = word_swap,
    [WORD_OVER] = word_over,
    [WORD_TUCK] = word_tuck,
    [WORD_ROT_BACK] = word_rot_back,
    [WORD_TEST] = word_nop,
    [WORD_LITERAL] = word_literal,
    [WORD_DEBUG] = word_nop,
    [WORD_LOOP] = word_nop,
    [WORD_LOOP_END] = word_loop_end,
    [WORD_DEFINE] = word_define,
    [WORD_DEFINE_END] = word_define_end,
    [WORD_CALL] = word_call,
    [WORD_EQUAL] = word_equal,
    [WORD_GREATER] = word_greater,
    [WORD_LESS] = word_less,
    [WORD_GREATER_EQUAL] = word_greater_equal,
    [WORD_LESS_EQUAL] = word_less_equal,
    [WORD_NOT_EQUAL] = word_not_equal,
    [WORD_AND] = word_and,
    [WORD_OR] = word_or,
    [WORD_SHL] = word_shl,
    [WORD_SHR] = word_shr,
    [WORD_XOR] = word_xor,
    [WORD_NOT] = word_not,
    [WORD_MOD] = word_mod,
    [WORD_SET] = word_set,
    [WORD_GET] = word_get,
    [WORD_CLOCK] = word_clock,
    [WORD_INCREMENT] = word_increment,
    [WORD_DECREMENT] = word_decrement,
    [WORD_GOTO] = word_goto,
    [WORD_PC] = word_nop,
    [WORD_SOLO] = word_nop,
    [WORD_MEMORY_MIN] = word_memory_min,
    [WORD_MEMORY_MAX] = word_memory_max,
    [WORD_WORDS_LIST] = word_nop,
    [WORD_STRING] = word_string,
    [WORD_STRING_END] = word_nop,
    [WORD_EMIT] = word_emit,
    [WORD_NEWLINE] = word_newline,
    [WORD_EXECUTE] = word_execute,
    [WORD_RETURN] = word_return,
    [WORD_END_OF_LIST_OVER] = word_nop,
};

#pragma region utils

struct word_t *words[] = {
    words0,
    words1,
    words2,
    words3,
    words4,
};

struct word_t word_none = {
    .info = {-1, 0, 0},
    .word = WORD_END,
    .name = "NULL",
    .func = NULL,
    .place = SOLO,
};

struct word_t find_word(enum parser_word_t word)
{
    for (int i = 0; i < 5; i++)
    {
        int id = 0;
        while (words[i][id].info.id != -1)
        {
            if (words[i][id].word == word)
            {
                return words[i][id];
            }
            id++;
        }
    }
    return word_none;
}

uint32_t get_hash(char *word, size_t len)
{
    switch (len)
    {
    case 1:
        return word[0];
        break;
    case 2:
        return word[0] << 8 | word[1];
        break;
    case 3:
        return word[0] << 16 | word[1] << 8 | word[2];
        break;
    case 4:
    default:
        return word[0] << 24 | word[1] << 16 | word[2] << 8 | word[3];
        break;
    }
    return 0;
}

void show_program(struct parser_t *parser)
{
    printf("Program:\n");
    int i = 1;
    while (parser->prog[i].op || parser->prog[i + 1].op)
    {
        printf("[%d] ", i);
        struct word_t w = find_word(parser->prog[i].op);
        if (w.info.id >= 0)
        {
            printf("%-8s <%d> ", w.name, parser->prog[i].len);
            if (w.place == SOLO)
            {
                printf("(%d) ", parser->prog[i].v.data.value.u);
            }
            for (uint8_t j = 0; j < 4; j++)
            {
                if (is_op(parser->prog[i], j))
                {
                    struct word_t w2 = find_word(parser->prog[i].v.op[j]);
                    if (w2.info.id >= 0)
                    {
                        printf("%s ", w2.name);
                    }
                    else
                    {
                        printf("%d? ", parser->prog[i].v.op[j]);
                    }
                }
            }
        }
        else
        {
            printf("?(%d) ", parser->prog[i].op);
        }
        printf("\n");
        i++;
    }
    printf("\n");
}

#pragma endregion

enum parser_mode_t hash(struct parser_t *parser, size_t len, char *word)
{
    if (len == 0)
    {
        return NONE;
    }
    char str[255] = {0};
    int i = 0;
    while (i < len)
    {
        str[i] = word[i];
        i++;
    }
    str[i] = '\0';
    DBG(2, "Hashing %d: %s\n", len, str);
    uint32_t hash = get_hash(str, len);
    // printf("Seeking: %d %d", len, hash);
    int wc = len > 4 ? 4 : len;
    int id = 0;
    while (words[wc][id].info.id != -1)
    {
        // printf("%d vs %d: %d vs %d\n", len, words[wc][id].len, hash, words[wc][id].hash);
        if (words[wc][id].info.len == len && words[wc][id].info.hash == hash)
        {
            if (words[wc][id].func == NULL)
            {
                printf("ERROR: word %s has no function\n", words[wc][id].name);
                return NONE;
            }
            DBG(2, "%d: %s\n", words[wc][id].info.id, words[wc][id].name);
            // words[wc][id].func(parser);
            if (words[wc][id].func != word_table[words[wc][id].word])
            {
                DBG(2, "Calling parser function: %s\n", words[wc][id].name);
                words[wc][id].func(parser);
            }
            else
            {
                add_word(parser, words[wc][id].word);
            }
            return NONE;
        }
        id++;
    }
    for (uint8_t i = 0; i < parser->uc; i++)
    {
        if (parser->uword[i].len == len && parser->uword[i].hash == hash)
        {
            uint8_t pc = add_word(parser, WORD_CALL);
            SET_DATA(pc, 0, parser->uword[i].id);
            return NONE;
        }
    }
    printf("%s?\n", str);
    return NONE;
}

void parse(struct parser_t *parser, char *buf)
{
    int len = 0;
    char word[255] = {0};
    bool loop = true;
    // printf("Parsing: %s\n", buf);
    int bc = 0;
    int64_t num = 0;
    parser->mode = NONE;
    bool minus = false;
    parser->pc = 1;

    while (loop)
    {
        if (parser->mode == STRING)
        {
            while (buf[bc] && buf[bc] != '"')
            {
                word[len++] = buf[bc++];
            }
            if (len > 0)
            {
                uint8_t n = 0;
                while (n < len)
                {
                    uint8_t pc = add_word(parser, WORD_STRING);
                    parser->prog[pc].len = 0;
                    while (parser->prog[pc].len < 4 && n < len && word[n])
                    {
                        // if end of input error
                        parser->prog[pc].v.op[parser->prog[pc].len++] = word[n++];
                    }
                }
                add_word(parser, WORD_STRING_END);
            }
            len = 0;
            parser->mode = NONE;
        }
        word[len++] = buf[bc];
        // printf("'%c' '%c' %d %d\n", buf[bc], word[len], parser->pc, len);
        switch (buf[bc])
        {
        case '$':
            if (len == 1)
            {
                parser->mode = HASH;
                len--;
            }
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            num *= 10;
            num += buf[bc] - '0';
            if (parser->mode == NONE)
            {
                parser->mode = NUMBER;
            }
            break;
        case '-':
            minus = true;
        case '+':
            num = 0;
            if (parser->mode == NUMBER)
            {
                parser->mode = NONE;
            }
            break;
        case ' ':
        case '\n':
        case '\r':
        case '\t':
        case '\0':
            // printf("Parsing %d\n", parser->mode);
            len--;
            if (buf[bc] == '\0')
            {
                DBG(1, "EOF\n");
                loop = false;
            }
            switch (parser->mode)
            {
            case NUMBER:
                if (minus)
                {
                    num *= -1;
                }
                if (num > STACK_MAX)
                {
                    printf("Error: number %lld too large\n", num);
                    exit(1);
                }
                if (num < STACK_MIN)
                {
                    printf("Error: number %lld too small\n", num);
                    exit(1);
                }
                add_word(parser, WORD_LITERAL);
                SET_UDATA(parser->pc - 1, num);
                num = 0;
                parser->mode = NONE;
                break;
            case NONE:
                hash(parser, len, word);
                break;
            case HASH:
                if (parser->prog[parser->pc - 1].op == WORD_DEFINE)
                {
                    DBG(2, "Defining user word: %.*s @ %d\n", len, word, parser->pc);
                    if (parser->uc > 0)
                    {
                        parser->uword[parser->uc - 1].len = len;
                        parser->uword[parser->uc - 1].hash = get_hash(word, len);
                    }
                }
                else
                {
                    add_word(parser, WORD_LITERAL);
                }
                SET_UDATA(parser->pc - 1, get_hash(word, len));
                SET_LEN(parser->pc - 1, len);
                parser->mode = NONE;
                break;
            case RETURN:
                printf("Returning on space\n");
                return;
            }
            len = 0;
            minus = false;
            break;
        }
        bc++;
        if (parser->mode == RETURN)
        {
            printf("Returning\n");
            return;
        }
    }
    parser->prog[parser->pc + 1].op = 0;
    parser->prog[parser->pc + 2].op = 0;
    parser->progsize = parser->pc + 1;
    parser->pc = 0;
    if (parser->skc > 0)
    {
        printf("Error: Unmatched control structure at end of program\n");
        exit(1);
    }
}

void parser_exec(struct parser_t *parser)
{
    if (parser->debug > 0)
    {
        show_program(parser);
    }
    if (PROG_PC == 0)
    {
        parser->pc = 1;
    }
    while (PROG_PC || parser->prog[parser->pc + 1].op)
    {
        CALL_WORD(PROG_PC);
        bool jumped = (parser->sub == 99);
        parser->sub = 0;
        if (!jumped)
        {
            while (parser->sub < 4 && is_op(parser->prog[parser->pc], parser->sub))
            {
                CALL_WORD(parser->prog[parser->pc].v.op[parser->sub]);
                parser->sub++;
            }
        }
        parser->pc++;
    }
    if (parser->debug > 1)
    {
        printf("\n");
        show_program(parser);
    }
}

int main(int argc, char *argv[])
{
    // printf("CELL%d DATA%d\n", sizeof(struct cell_t), sizeof(struct data_t), sizeof(struct data_t));

    if ((argc == 2 && get_hash(argv[1], 6) == HASH4('-', '-', 'h', 'e')) || (argc == 1))
    {
        printf("Usage: %s [options] '<program>'\n", argv[0]);
        printf("Use 'word' to list available words.\n");
        printf("Options:\n");
        printf("\t--help\tShow this help message and exit\n");
        return 0;
    }
    struct parser_t parser;
    parser_init(&parser);
    for (uint8_t i = 1; i < argc; i++)
    {
        parse(&parser, argv[i]);
    }
    printf("\n");
    if (parser.debug > 0)
    {
        printf("Program size: %d\n", parser.progsize);
    }
    parser_exec(&parser);

    printf("\n");
    if (parser.debug > 0)
    {
        for (uint8_t i = 0; i < parser.uc; i++)
        {
            printf("User word %d: %d %d @ %d\n", i, parser.uword[i].len, parser.uword[i].hash, parser.uword[i].id);
        }
        printf("\n");
    }
    return 0;
}