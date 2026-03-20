Import("env")

# Flag specifici per Cortex-M4 con FPU Single Precision
flags = [
    "-mfloat-abi=hard",
    "-mfpu=fpv4-sp-d16"
]

# Aggiunge i flag sia alla compilazione (CCFLAGS) che al linker (LINKFLAGS)
env.Append(CCFLAGS=flags, LINKFLAGS=flags)