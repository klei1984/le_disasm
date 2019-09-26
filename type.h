#ifndef LE_DISASM_TYPE_H_
#define LE_DISASM_TYPE_H_

enum Type {
    UNKNOWN,   /* region */
    CODE,      /* region */
    DATA,      /* region, label */
    SWITCH,    /* region, label */
    ALIGNMENT, /* region */
    JUMP,      /* label */
    FUNCTION,  /* label */
    CASE,      /* label */
    FUNC_GUESS /* label; either callback or data in code segment */
};

enum Bitness { BITNESS_32BIT, BITNESS_16BIT };

std::ostream &printAddress(std::ostream &os, uint32_t address, const char *prefix = "0x");
std::ostream &operator<<(std::ostream &os, Type type);

#endif /* LE_DISASM_TYPE_H_ */
