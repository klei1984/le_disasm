#ifndef LE_DISASM_EMITTER_H_
#define LE_DISASM_EMITTER_H_

#include "analyzer.h"

class Emitter {
    LinearExecutable &lx;
    Image &img;
    Regions &regions;
    std::map<uint32_t, Type> &labelTypes;
    SymbolMap *map;

    static int get_indent(Type type);
    std::ostream &print_typed_address(std::ostream &os, uint32_t address, Type type);
    std::ostream &print_label(uint32_t address, Type type, char const *prefix = "");
    bool data_is_address(const ImageObject &obj, uint32_t addr, size_t len);
    bool data_is_zeros(const ImageObject &obj, uint32_t addr, size_t len, size_t &rlen);
    bool data_is_string(const ImageObject &obj, uint32_t addr, size_t len, size_t &rlen, bool &zero_terminated);
    static void print_escaped_string(const uint8_t *data, size_t len);
    static void complete_string_quoting(int &bytes_in_line, int resetTo = 0);
    size_t get_len(const Region &reg, const ImageObject &obj, std::map<uint32_t, uint32_t> &fups,
                   std::map<uint32_t, uint32_t>::const_iterator &itr, uint32_t address);
    void print_data_after_fixup(const ImageObject &obj, uint32_t &address, size_t len, int &bytes_in_line);
    void print_eip();
    void print_code();
    void print_region(const Region &reg);
    void print_unknown_type_region(const Region &reg);
    std::string replace_addresses_with_labels(Insn &inst);
    void print_instruction(Insn &inst);
    void print_code_type_region(const Region &reg);
    void print_data_type_region(const Region &reg);
    void print_switch_type_region(const Region &reg);
    void print_alignment_type_region(const Region &reg);
    void print_changed_section_type(const Region &reg, const Region *const reg_prev, Type &section);

public:
    Emitter(LinearExecutable &lx, Image &img, Analyzer &anal, SymbolMap *map);
    virtual ~Emitter();
    void run();
};

#endif /* LE_DISASM_EMITTER_H_ */
