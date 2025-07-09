/** 
 * @file libgpx.h 
 * @author @adamml
 * @date 2022-08-04
 * @brief C library for handling GPX Exchange Format data
 * */
#ifndef COM_GITHUB_ADAMML__LIBGPX__LIBGPX__H
#define COM_GITHUB_ADAMML__LIBGPX__LIBGPX__H

#include <libxml/parser.h>
#include <libxml/tree.h>

#define LIBGPX__XML_STRING_ALLOCATED_LENGTH 65 /**< Used to cinfigure the amount of memory allocated to character arrays in result structures for storing the GPX data */

#define LIBGPX__COLLECTION_TYPE_RTE 0
#define LIBGPX__COLLECTION_TYPE_TRK 1
#define LIBGPX__COLLECTION_TYPE_TRKSEG 2

#ifdef __cplusplus
extern "C" {
#endif

typedef struct libgpx_sBounndingBox{
    float max_ele;
    float min_ele;
    float max_lat;
    float min_lat;
    float max_lon;
    float min_lon;
} libgpx_BoundingBox;

typedef struct libgpx_sGPXLink{
    char href[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    char text[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    char type[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    int member_of;
} libgpx_GPXLink;

typedef struct libgpx_sGPXPoint{
    size_t id;
    int member_of;
    double ageofdgpsdata;
    char comment[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    char description[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    int dgpsid;
    double elevation;
    char fix[5];
    double geoidheight;
    double hdop;
    double latitude;
    double longitude;
    double magvar;
    char name[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    double pdop;
    int satellite;
    char source[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    char symbol[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    char time[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    char type[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    double vdop;
} libgpx_GPXPoint;

typedef struct libgpx_sGPXPointCollection {
    size_t id;
    unsigned int collection_type:2;
    int member_of;
    char name[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    char type[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
} libgpx_GPXPointCollection;

typedef struct libgpx_sGPXEmail {
    char id[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    char domain[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
} libgpx_GPXEmail;

typedef struct libgpx_sGPXAuthor {
    char name[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    libgpx_GPXEmail email;
} libgpx_GPXAuthor;

typedef struct libgpx_sGPXCopyright {
    char author[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    unsigned int year:12;
    char license[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
} libgpx_GPXCopyright;

typedef struct libgpx_sGPXMetadata {
    char name[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    libgpx_GPXAuthor author;
    libgpx_GPXCopyright copyright;
} libgpx_GPXMetadata;

/** 
 * The top-level structure, collecting together all GPX information
*/
typedef struct libgpx_sGPX{
    char creator[LIBGPX__XML_STRING_ALLOCATED_LENGTH];
    size_t n_GPXPointCollection;
    size_t n_GPXPoint;
    size_t n_GPXLink;
    libgpx_GPXMetadata metadata;
    libgpx_GPXPointCollection *collections; /**< Stores routes, tracks and track segements */
    libgpx_GPXLink *links; /**< Stores links */
    libgpx_GPXPoint *points; /**< Stores route points, track points and way points */
    float version;
} libgpx_GPX;

/** Counts the individuals of various types in a GPX file
 * 
 * @param gpx   A pointer to a character array containing the contents of a GPX file
 * @param sgpx A pointer to a libgpx_GPX structure
 * @return An integer, 1 on success, a negative integer on failure
 * */
int libgpx_count_gpx_types(char* gpx, libgpx_GPX* sgpx);

void libgpx_parse_gpx(unsigned char* gpx, libgpx_GPX* sgpx);

void libgpx_bounding_box_from_gpx_object(libgpx_GPX* sgpx, libgpx_BoundingBox* box);

#ifdef __cplusplus
}
#endif
#endif /** libgpx.h */