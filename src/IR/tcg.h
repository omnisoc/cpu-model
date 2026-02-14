#ifndef IR_TCG_H
#define IR_TCG_H
#include <vector>
#include <cstdint>
namespace IR {
enum class TCGOp { OP_MOVI, OP_ADD, OP_ADDI, OP_SUB, OP_EXIT };
struct TCGInstruction { TCGOp op; uint32_t rd; uint32_t rs1; uint32_t rs2; };
struct TCGBlock { std::vector<TCGInstruction> insts; };
}
#endif
