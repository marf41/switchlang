enum parser_mode_t
{
    NONE,
    NUMBER,
    HASH,
    STRING,
    RETURN,
};

enum __attribute__((__packed__)) parser_word_t
{
    WORD_END = 0,
    WORD_NOP,
    WORD_DOT,
    WORD_ADD,
    WORD_SUBTRACT,
    WORD_MULTIPLY,
    WORD_DIVIDE,
    WORD_PRINT_STACK,
    WORD_IF,
    WORD_IF_ELSE,
    WORD_IF_END,
    WORD_ABS,
    WORD_DUP,
    WORD_NIP,
    WORD_ROT,
    WORD_DROP,
    WORD_SWAP,
    WORD_OVER,
    WORD_TUCK,
    WORD_ROT_BACK,
    WORD_TEST,
    WORD_LITERAL,
    WORD_DEBUG,
    WORD_LOOP,
    WORD_LOOP_END,
    WORD_DEFINE,
    WORD_DEFINE_END,
    WORD_CALL,
    WORD_EQUAL,
    WORD_GREATER,
    WORD_LESS,
    WORD_GREATER_EQUAL,
    WORD_LESS_EQUAL,
    WORD_NOT_EQUAL,
    WORD_AND,
    WORD_OR,
    WORD_SHL,
    WORD_SHR,
    WORD_XOR,
    WORD_NOT,
    WORD_MOD,
    WORD_SET,
    WORD_GET,
    WORD_CLOCK,
    WORD_INCREMENT,
    WORD_DECREMENT,
    WORD_GOTO,
    WORD_PC,
    WORD_SOLO,
    WORD_MEMORY_MIN,
    WORD_MEMORY_MAX,
    WORD_WORDS_LIST,
    WORD_STRING,
    WORD_STRING_END,
    WORD_EMIT,
    WORD_NEWLINE,
    WORD_END_OF_LIST_OVER
};

enum __attribute__((__packed__)) word_place_t
{
    ANY,
    SOLO,
    LAST
};

enum __attribute__((__packed__)) data_type_t
{
    TYPE_UNKOWN,
    TYPE_UINT,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
};