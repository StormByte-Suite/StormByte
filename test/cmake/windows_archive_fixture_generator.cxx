#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

void WriteUint32(std::ofstream& out, std::uint32_t value) {
	const char bytes[] = {
		static_cast<char>(value >> 24),
		static_cast<char>(value >> 16),
		static_cast<char>(value >> 8),
		static_cast<char>(value)
	};
	out.write(bytes, sizeof(bytes));
}

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cerr << "Expected an archive path and at least one symbol name\n";
		return 1;
	}

	const bool missing_names = argc == 3 && std::string(argv[2]) == "--missing-names";
	const bool unterminated = argc == 3 && std::string(argv[2]) == "--unterminated";
	const auto count = static_cast<std::uint32_t>(argc - 2);
	std::size_t index_size = 4 + 4 * count;
	for (int i = 2; i < argc; ++i) {
		if (!*argv[i]) {
			std::cerr << "Fixture symbol must not be empty\n";
			return 1;
		}
		if (!missing_names)
			index_size += std::string(argv[i]).size() + (unterminated ? 0 : 1);
	}
	const std::string size = std::to_string(index_size);
	if (size.size() > 10) {
		std::cerr << "Fixture symbol is too long\n";
		return 1;
	}

	std::ofstream out(argv[1], std::ios::binary | std::ios::trunc);
	if (!out) {
		std::cerr << "Could not create fixture archive: " << argv[1] << '\n';
		return 1;
	}

	out << "!<arch>\n/               0           0     0     0       "
		<< size << std::string(10 - size.size(), ' ') << "`\n";
	WriteUint32(out, count);
	for (std::uint32_t i = 0; i < count; ++i)
		WriteUint32(out, 0);
	for (int i = 2; i < argc; ++i) {
		if (!missing_names) {
			out << argv[i];
			if (!unterminated)
				out.put('\0');
		}
	}
	out << "__IMPORT_DESCRIPTOR_decoy_outside_linker_index";
	if (!out) {
		std::cerr << "Could not write fixture archive: " << argv[1] << '\n';
		return 1;
	}
}
