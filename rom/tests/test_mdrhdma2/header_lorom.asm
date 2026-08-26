org $00ffc0
header_lorom() {
  db 'SNES TEST IMAGE ~BYUU'
  db $30   ;lorom
  db $02   ;rom+sram
  db $08   ;2mbit rom
  db $05   ;128kbit sram
  db $00   ;japan
  db $00   ;no developer
  db $00   ;version 1.0
  dw $0000 ;inverse checksum
  dw $ffff ;checksum

  dw $ffff
  dw $ffff
  dw $ffff
  dw $f400 ;brk
  dw $ffff
  dw $f000 ;nmi
  dw $ffff
  dw $f800 ;irq
  dw $ffff
  dw $ffff
  dw $fc00 ;cop
  dw $ffff
  dw $ffff
  dw $f000 ;nmi in emulation-mode
  dw $8000 ;reset
  dw $f800 ;irq+brk in emulation-mode
}
