#pragma once

#define VGM_HEADER_FIELDS \
	VGM_HEADER_OFFSET(0x04, eof_offset, 0x100) \
	VGM_HEADER_FIELD32(0x08, version, 0x100) \
	VGM_HEADER_FIELD32(0x0C, sn76489_clock, 0x100) \
	VGM_HEADER_FIELD32(0x10, ym2413_clock, 0x100) \
	VGM_HEADER_OFFSET(0x14, gd3_offset, 0x100) \
	VGM_HEADER_FIELD32(0x18, total_samples, 0x100) \
	VGM_HEADER_OFFSET(0x1C, loop_offset, 0x100) \
	VGM_HEADER_FIELD32(0x20, loop_samples, 0x100) \
	VGM_HEADER_FIELD32(0x24, rate, 0x101) \
	VGM_HEADER_FIELD16(0x28, sn76489_feedback, 0x110) \
	VGM_HEADER_FIELD8(0x2A, sn76489_shift_register_width, 0x110) \
	VGM_HEADER_FIELD8(0x2B, sn76489_flags, 0x151) \
	VGM_HEADER_FIELD32(0x2C, ym2612_clock, 0x110) \
	VGM_HEADER_FIELD32(0x30, ym2151_clock, 0x110) \
	VGM_HEADER_OFFSET(0x34, vgm_data_offset, 0x150) \
	VGM_HEADER_FIELD32(0x38, sega_pcm_clock, 0x151) \
	VGM_HEADER_FIELD32(0x3C, sega_pcm_interface_register, 0x151) \
	VGM_HEADER_FIELD32(0x40, rf5c68_clock, 0x151) \
	VGM_HEADER_FIELD32(0x44, ym2203_clock, 0x151) \
	VGM_HEADER_FIELD32(0x48, ym2608_clock, 0x151) \
	VGM_HEADER_FIELD32(0x4C, ym2610_clock, 0x151) \
	VGM_HEADER_FIELD32(0x50, ym3812_clock, 0x151) \
	VGM_HEADER_FIELD32(0x54, ym3526_clock, 0x151) \
	VGM_HEADER_FIELD32(0x58, y8950_clock, 0x151) \
	VGM_HEADER_FIELD32(0x5C, ymf262_clock, 0x151) \
	VGM_HEADER_FIELD32(0x60, ymf278b_clock, 0x151) \
	VGM_HEADER_FIELD32(0x64, ymf271_clock, 0x151) \
	VGM_HEADER_FIELD32(0x68, ymz280b_clock, 0x151) \
	VGM_HEADER_FIELD32(0x6C, rf5c164_clock, 0x151) \
	VGM_HEADER_FIELD32(0x70, pwm_clock, 0x151) \
	VGM_HEADER_FIELD32(0x74, ay8910_clock, 0x151) \
	VGM_HEADER_FIELD8(0x78, ay8910_chip_type, 0x151) \
	VGM_HEADER_FIELD8(0x79, ay8910_flags, 0x151) \
	VGM_HEADER_FIELD8(0x7A, ym2203_flags, 0x151) \
	VGM_HEADER_FIELD8(0x7B, ym2608_flags, 0x151) \
	VGM_HEADER_FIELD8(0x7C, volume_modifier, 0x160) \
	VGM_HEADER_FIELD8(0x7E, loop_base, 0x160) \
	VGM_HEADER_FIELD8(0x7F, loop_modifier, 0x151) \
	VGM_HEADER_FIELD32(0x80, gameboy_dmg_clock, 0x161) \
	VGM_HEADER_FIELD32(0x84, nes_apu_clock, 0x161) \
	VGM_HEADER_FIELD32(0x88, multipcm_clock, 0x161) \
	VGM_HEADER_FIELD32(0x8C, upd7759_clock, 0x161) \
	VGM_HEADER_FIELD32(0x90, okim6258_clock, 0x161) \
	VGM_HEADER_FIELD8(0x94, okim6258_flags, 0x161) \
	VGM_HEADER_FIELD8(0x95, k054539_flags, 0x161) \
	VGM_HEADER_FIELD8(0x96, c140_chip_type, 0x161) \
	VGM_HEADER_FIELD32(0x98, okim6295_clock, 0x161) \
	VGM_HEADER_FIELD32(0x9C, k051649_clock, 0x161) \
	VGM_HEADER_FIELD32(0xA0, k054539_clock, 0x161) \
	VGM_HEADER_FIELD32(0xA4, huc6280_clock, 0x161) \
	VGM_HEADER_FIELD32(0xA8, c140_clock, 0x161) \
	VGM_HEADER_FIELD32(0xAC, k053260_clock, 0x161) \
	VGM_HEADER_FIELD32(0xB0, pokey_clock, 0x161) \
	VGM_HEADER_FIELD32(0xB4, qsound_clock, 0x161) \
	VGM_HEADER_FIELD32(0xB8, scsp_clock, 0x171) \
	VGM_HEADER_OFFSET(0xBC, extra_header_offset, 0x170) \
	VGM_HEADER_FIELD32(0xC0, wonderswan_clock, 0x171) \
	VGM_HEADER_FIELD32(0xC4, vsu_clock, 0x171) \
	VGM_HEADER_FIELD32(0xC8, saa1099_clock, 0x171) \
	VGM_HEADER_FIELD32(0xCC, es5503_clock, 0x171) \
	VGM_HEADER_FIELD32(0xD0, es5505_clock, 0x171) \
	VGM_HEADER_FIELD8(0xD4, es5503_amount_of_output_channels, 0x171) \
	VGM_HEADER_FIELD8(0xD5, es5505_es5506_amount_of_output_channels, 0x171) \
	VGM_HEADER_FIELD8(0xD6, c352_clock_divider, 0x171) \
	VGM_HEADER_FIELD32(0xD8, x1_010_clock, 0x171) \
	VGM_HEADER_FIELD32(0xDC, c352_clock, 0x171) \
	VGM_HEADER_FIELD32(0xE0, ga20_clock, 0x171)

struct vgm_header {
#define VGM_HEADER_FIELD32(ofs, name, version) uint32_t name;
#define VGM_HEADER_FIELD16(ofs, name, version) uint16_t name;
#define VGM_HEADER_FIELD8(ofs, name, version) uint8_t name;
#define VGM_HEADER_OFFSET(ofs, name, version) uint32_t name;
VGM_HEADER_FIELDS
#undef VGM_HEADER_OFFSET
#undef VGM_HEADER_FIELD8
#undef VGM_HEADER_FIELD16
#undef VGM_HEADER_FIELD32
};
