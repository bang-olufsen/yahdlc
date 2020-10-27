/**
 * @file fcs.h
 */

#ifndef FCS_H
#define FCS_H

#ifdef CRC32
    #define FCS_INIT_VALUE 0xFFFFFFFF /* FCS initialization value. */
    #define FCS_GOOD_VALUE 0xDEBB20E3 /* FCS value for valid frames. */
    #define FCS_INVERT_MASK 0xFFFFFFFF /* Invert the FCS value accordingly to the specification */
    #define FCS_SIZE unsigned int
#else
    #define FCS_INIT_VALUE 0xFFFF /* FCS initialization value. */
    #define FCS_GOOD_VALUE 0xF0B8 /* FCS value for valid frames. */
    #define FCS_INVERT_MASK 0xFFFF /* Invert the FCS value accordingly to the specification */
    #define FCS_SIZE unsigned short
#endif


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
FCS_SIZE calc_fcs(FCS_SIZE fcs, unsigned char value);

#ifdef __cplusplus
}
#endif

#endif
