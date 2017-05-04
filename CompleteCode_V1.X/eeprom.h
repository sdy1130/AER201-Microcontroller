#ifndef EEPROM_H
#define	EEPROM_H

signed char Eeprom_ReadByte(signed char address);
void Eeprom_WriteByte(signed char address, signed char data);

#endif	/* EEPROM_H */