/** Generate map file
 *
 *   Generate a map file that complies to the following format:
 *        \1 symbol_name   \2 type   \3 start_address   \4 region_size
 */

#include <idc.idc>

#define True 1
#define False !True

#define has_any_name(F) ((F & FF_ANYNAME) != 0)
#define is_function(EA) (get_func_flags(EA) != -1)
#define is_function_start(F) (is_code(F) && ((F & FF_FUNC) != 0) && ((F & FF_IMMD) == 0))
#define is_lut_entry(F, B) (is_off0(F) && (((B == 0) && (is_word(F))) || ((B == 1) && (is_dword(F)))))

static ProcessAlign(Address, Handle) {
	return next_head(Address, get_segm_end(Address));
}

static TryLUT(Address) {
	auto Bitness;
	auto EndAddress;
	auto Flags;

	Flags = GetFlags(Address);
	Bitness = get_segm_attr(Address, SEGATTR_BITNESS);

	if (has_any_name(Flags) && has_xref(Flags) && is_lut_entry(Flags, Bitness)) {
		auto NextAddress;
		auto ThisAddress;
		auto NextFlags;
		auto WordSize;
		auto LutSize;
		auto ArraySize;

		ThisAddress = Address;
		WordSize = (Bitness == 0) ? 2 : 4;

		// check whether LUT is an array
		EndAddress = next_head(Address, get_segm_end(Address));
		ArraySize = EndAddress - Address;
		if ((ArraySize > WordSize) && ((ArraySize % WordSize) == 0)) {
			LutSize = ArraySize;
		} else {
			LutSize = WordSize;

			// start of LUT is found, now search for further LUT entires
			for (;;) {
				if ((ThisAddress + WordSize) >= get_segm_end(Address)) break;
				NextAddress = ThisAddress + WordSize;
				NextFlags = GetFlags(NextAddress);

				if (!has_any_name(NextFlags) && !has_xref(NextFlags) && is_lut_entry(NextFlags, Bitness)) {
					ThisAddress = NextAddress;
					LutSize = LutSize + WordSize;
				} else break;
			}
		}

		return LutSize;
	}

	return 0;
}

static ProcessData(Address, Flags, Handle) {
	auto EndAddress;

	EndAddress = next_head(Address, get_segm_end(Address));
	if (EndAddress == -1) {
		EndAddress = get_segm_end(Address);
	}

	if (has_any_name(Flags)) {
		auto Class;
		auto Size;
		auto Name;

		if (is_strlit(Flags)) {
			Name = get_name(Address);
			Size = EndAddress - Address;
			Class = "ASCII";
			fprintf(Handle, "%s %s %08X %08X\n", Name, Class, Address, Size);
		} else {
			Size = TryLUT(Address);
			Name = get_name(Address);
			if (Size) {
				Class = "LUT";
				fprintf(Handle, "%s %s %08X %08X\n", Name, Class, Address, Size);
				EndAddress = Address + Size;
			} else {
				Name = get_name(Address);
				Size = EndAddress - Address;
				Class = "DATA";
				fprintf(Handle, "%s %s %08X %08X\n", Name, Class, Address, Size);
			}
		}
	}

	return EndAddress;
}

static ProcessFunction(Address, Handle) {
	auto Name;
	auto Class;
	auto Bitness;
	auto Type;
	auto Size;
	auto StartAddress;
	auto EndAddress;

	Bitness = get_segm_attr(Address, SEGATTR_BITNESS);
	Type = get_segm_attr(Address, SEGATTR_TYPE);

	if (Type == SEG_CODE) {
		if (Bitness == 0) {
			Class = "FUNC16";
		} else if (Bitness == 1) {
			Class = "FUNC32";
		}
	}

	Name = get_func_name(Address);
	StartAddress = get_fchunk_attr(Address, FUNCATTR_START);
	EndAddress = get_fchunk_attr(Address, FUNCATTR_END);
	Size = EndAddress - StartAddress;

	if (StartAddress == first_func_chunk(Address)) {
		fprintf(Handle, "%s %s %08X %08X\n", Name, Class, Address, Size);
	}

	return EndAddress;
}

static ProcessSegment(SegmentAddress, Handle) {
	auto StartAddress;
	auto EndAddress;
	auto Address;
	auto Flags;

	StartAddress = get_segm_start(SegmentAddress);
	EndAddress = get_segm_end(SegmentAddress) -1;

	Address = StartAddress;
	msg("Processing segment %s\n", get_segm_name(StartAddress));

	for (;;) {

		if (Address >= EndAddress) {
			return;
		}

		Flags = GetFlags(Address);

		if (is_align(Flags)){
			Address = ProcessAlign(Address, Handle);
		} else if (is_function(Address) && has_any_name(Flags)){
			Address = ProcessFunction(Address, Handle);
		} else if (is_data(Flags) || has_value(Flags)) {
			Address = ProcessData(Address, Flags, Handle);
		} else if (1) {
			msg("Unknown found at %08X, aborting...\n", Address);
			return;
		}
	}
}

static main(void) {
	auto Address;
	auto File;
	auto Handle;

	File = ask_file(Handle, "mapfile.map", "*.map");
	Handle = fopen(File, "wt");
	if (Handle == 0) { msg("\nFailed to open file %s\n", File); return; }

	msg("\nGenerate map file %s\n", File);
	for (Address = next_head(0, -1); Address != -1; Address = next_head(get_segm_end(Address), -1)) {
		ProcessSegment(Address, Handle);
	}

	fclose(Handle);
	msg("Done\n");
}
