/**
 * @file fcs16.h
 */

#ifndef FCS16_H
#define FCS16_H

/** FCS initialization value. */
#define FCS16_INIT_VALUE 0xFFFF

/** FCS value for valid frames. */
#define FCS16_GOOD_VALUE 0xF0B8

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Calculates a new FCS based on the current value and value of data.
 *
 * @param fcs Current FCS value
 * @param value The value to be added
 * @returns Calculated FCS value
 */
unsigned short fcs16(unsigned short fcs, unsigned char value);

#ifdef __cplusplus
}
#endif

#endif
