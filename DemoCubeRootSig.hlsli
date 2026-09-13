#ifndef ROOT_SIGNATURE_HLSLI
#define ROOT_SIGNATURE_HLSLI

#define ROOT_SIGNATURE \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | " \
    "DENY_HULL_SHADER_ROOT_ACCESS | " \
    "DENY_DOMAIN_SHADER_ROOT_ACCESS | " \
    "DENY_GEOMETRY_SHADER_ROOT_ACCESS | " \
    "DENY_PIXEL_SHADER_ROOT_ACCESS), " \
    "RootConstants(num32BitConstants=16, b0, visibility=SHADER_VISIBILITY_VERTEX), " \
    "RootConstants(num32BitConstants=16, b1, visibility=SHADER_VISIBILITY_VERTEX), " \
    "RootConstants(num32BitConstants=16, b2, visibility=SHADER_VISIBILITY_VERTEX)"

#endif