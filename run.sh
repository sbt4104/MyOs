#assemble boot.s file
as --32 boot.s -o boot.o

#compile kernel.c file
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
#g++ -m32 -c kernel.cpp -o kernel.o -ffreestanding -O2 -Wall -Wextra

#linking the kernel with kernel.o and boot.o files
ld -m elf_i386 -T linker.ld kernel.o boot.o -o mykernel.bin -nostdlib

#check mykernel.bin file is x86 multiboot file or not
grub-file --is-x86-multiboot mykernel.bin

#building the iso file
mkdir -p isodir/boot/grub
cp mykernel.bin isodir/boot/mykernel.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o mykernel.iso isodir

#run it in qemu
qemu-system-x86_64 -m 512 -cdrom mykernel.iso
