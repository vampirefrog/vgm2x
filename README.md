VGM extraction and conversion tools
===================================

Some tools to extract MIDI and 4-op FM voices from [.VGM files](https://vgmrips.net/wiki/VGM_File_Format).

Conversion tools
----------------

| 2 ⮣| vgm | gym | dro | s98 | txt | opm | dmp | tfi | ins | y12 | mid |
| ---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| vgm | ⛔  | 🔲  | 🔲  | 🔲  | 🔲  | ☑️  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  |
| gym | 🔲  | ⛔  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  |
| dro | 🔲  | 🔲  | ⛔  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  |
| s98 | 🔲  | 🔲  | 🔲  | ⛔  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  | 🔲  |
| txt | 🔲  | 🔲  | 🔲  | 🔲  | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  |
| opm | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | 🔲  | 🔲  | 🔲  | 🔲  | ⛔  |
| dmp | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | 🔲  | ⛔  | 🔲  | 🔲  | 🔲  | ⛔  |
| tfi | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | 🔲  | 🔲  | ⛔  | 🔲  | 🔲  | ⛔  |
| ins | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | 🔲  | 🔲  | 🔲  | ⛔  | 🔲  | ⛔  |
| y12 | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | 🔲  | 🔲  | 🔲  | 🔲  | ⛔  | ⛔  |
| mid | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  | ⛔  |

Legend:
* ⛔ Not applicable
* ☑️ Implemented
* 🔲 Unimplemented

vgm2opm
-------
Similar to Shiru's vgm2opm, it extracts 4-op FM voices from vgm files. Unlike Shiru's vgm2opm, it supports more chips, such as the OPN\* series and OPM, and limited support for OPL\*. It uses roughly the same algorithm to compact duplicate voices.

Resources:
----------
* [vgm2pre](https://www.deflemask.com/forum/general/vgm2pre-vgm-preset-dumper-(open-beta!)/)
* [S98](https://vgmrips.net/wiki/S98_File_Format)
