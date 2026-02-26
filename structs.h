struct data_t
{
    union
    {
        uint32_t u;
        int32_t i;
        float f;
    } value;
    enum data_type_t type;
    uint8_t len;
    bool is_const : 1;
};

struct cell_t
{
    enum parser_word_t op;
    bool is_op0 : 1;
    bool is_op1 : 1;
    bool is_op2 : 1;
    bool is_op3 : 1;
    uint8_t dc : 2;
    enum word_place_t place;
    uint8_t len;
    union
    {
        enum parser_word_t op[4];
        struct data_t data;
    } v;
};

struct word_info_t
{
    int16_t id;
    uint8_t len;
    uint32_t hash;
};

struct parser_t
{
    char *buf;
    stack_t stack[100];
    uint8_t sc;
    struct cell_t prog[MAX_PROG];
    uint8_t pc;
    uint8_t sub;
    uint8_t skip[16];
    uint8_t skc;
    stack_t ret[16];
    uint8_t rc;
    struct word_info_t uword[MAX_USER];
    uint8_t uc;

    struct data_t data[MAX_DATA];

    uint8_t progsize;
    uint8_t debug;
    enum parser_mode_t mode;
};

struct word_t
{
    struct word_info_t info;
    enum parser_word_t word;
    char name[16];
    void (*func)(struct parser_t *parser);
    enum word_place_t place;
#ifdef INCLUDE_WORD_TESTS
    bool test_hide;
    char test_before[255];
    char test_after[255];
#endif
};
