# slimloader
Miniature bootloader for Ditto Mini, hooking the BIOS's multicart functionality to fit within another ROM.

This allows for more ROM slots than zoranc's bootloader; up to 4 retail ROMs and many more homebrew.

Automatic ROM patching is **not** included, as there are only a few bytes spare for extra code.
 
## Building
`mk88` to build `slimloader.min`, `mk88 combined` to build a multiboot ROM using `in.min`, `in2.min`, `in3.min`, `in4.min` and `slimloader.min`.

Designed to be built on Windows using the C88 toolchain; I'm not sure how well it'll run under Wine, and you might need to redefine RM, RMDIR and MKDIR for it to work.
