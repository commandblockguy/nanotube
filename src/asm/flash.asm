	def _flash_write

heapBot			:= 0D1887Ch

;void flash_write(void *flash_loc, void *data, size_t size)
_flash_write:
	call	flash_unlock
	ld	iy,0
	add	iy,sp
	push	ix
	xor	a,a
	sbc	hl,hl
	ld	bc,(iy+9)
	sbc	hl,bc
	jr	z,_flash_write.zero_size
	ld  hl,(iy+6)
	ld  de,(iy+3)
	push	iy
	ld	iy,13631616
	call	$0002E0 ; ti.WriteFlash
	pop iy
_flash_write.zero_size:
	call	flash_lock
	pop ix
	ret

write_port:
	ld	de,$C979ED
	ld	hl,heapBot - 3
	ld	(hl),de
	jp	(hl)

read_port:
	ld	de,$C978ED
	ld	hl,heapBot - 3
	ld	(hl),de
	jp	(hl)

flash_unlock:
	ld	bc,$24
	ld	a,$8c
	call	write_port
	ld	bc,$06
	call	read_port
	or	a,4
	call	write_port
	ld	bc,$28
	ld	a,$4
	jp	write_port

flash_lock:
	ld	bc,$28
	xor	a,a
	call	write_port
	ld	bc,$06
	call	read_port
	res	2,a
	call	write_port
	ld	bc,$24
	ld	a,$88
	jp	write_port

write_bytes:
	call	write_byte
	inc hl
	inc de
	djnz	write_bytes
	ret

write_byte:
	ld  a,$AA
	ld  ($AAA),a
	ld  a,$55
	ld  ($555),a
	ld  a,$A0
	ld  ($AAA),a
	ld  a,(hl)
	ld  (de),a
	ret