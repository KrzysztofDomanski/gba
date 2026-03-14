import struct

print("Compiling Hello Pixel ROM...")

# ARM7TDMI Machine Code
opcodes = [
    # Setup
    0xE3A00406, # 0x08000000: MOV R0, #0x06000000 (VRAM Base Address)
    0xE3A0101F, # 0x08000004: MOV R1, #0x001F     (Color: Pure Red)
    0xE3A02C96, # 0x08000008: MOV R2, #0x9600     (Limit: 38,400 pixels)
    0xE3A03000, # 0x0800000C: MOV R3, #0          (Counter: 0)

    # --- LOOP START ---
    0xE1C010B0, # 0x08000010: STRH R1, [R0]       (Draw pixel to VRAM)
    0xE2800002, # 0x08000014: ADD R0, R0, #2      (Advance VRAM pointer by 1 halfword)
    0xE2833001, # 0x08000018: ADD R3, R3, #1      (Increment our pixel counter)
    0xE1530002, # 0x0800001C: CMP R3, R2          (Check if Counter == 38,400)

    # Branch offset math: Target is 0x10. PC is at 0x28. Offset is -24 bytes (-6 words).
    0x1AFFFFFA, # 0x08000020: BNE Loop            (If Not Equal, jump back to 0x10)
    # --- LOOP END ---

    # Safe exit state. Offset is -8 bytes (-2 words) to jump to itself forever.
    0xEAFFFFFE  # 0x08000024: B End               (Infinite loop)
]

# Write to a binary file using Little-Endian unsigned integers ('<I')
with open("hello_pixel.gba", "wb") as f:
    for opcode in opcodes:
        f.write(struct.pack('<I', opcode))

print("Success: hello_pixel.gba generated!")
