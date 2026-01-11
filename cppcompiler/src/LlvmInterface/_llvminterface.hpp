#pragma once
#include "../module.hpp"
namespace LlvmInterface {
void init();
void assemble(const char* filepath, IrFile& file);
void compile(const char* filepath, IrFile& file);
void link(const char* filepath, ObjFile& file);
} // namespace LlvmInterface
