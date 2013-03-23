#ifndef MUSICXML_H
#define MUSICXML_H

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_histogram.h>

#define DIVISIONS 96 // this is the length of a quarter-note
#define MAX_STRING 64
#define NOTESCALEFACTOR 24 // Assumes a 16th-note granularity, should be 1/4 of DIVISIONS
#define NUMMAX 30
#define AVG_WINDOW 300
#define THRESHOLD_FACTOR .31

typedef struct
{
    int notes[12];
    int id;
} Type;

typedef struct
{
    int function;
    Type type;
    float duration;
} Section;

typedef struct
{
    int key;
    int length;
    Section section[1000];
} Composition;

typedef struct
{
    int octave;
    int step;
    int alter;
    int accidental;
} Note;

typedef struct
{
    int notes[88];
    int type_id;
} Range; 

typedef struct
{
    int key;
    int num;
    int beats;
    int beat_type;
} MeasureAttribute;

typedef struct part
{
    int note_num;
    int duration;
    int staff;
    int rest;
    struct part* next;
} Part;

typedef struct rhythm
{
    int divisions;
    struct rhythm* next;
} Rhythm;

typedef struct harmony
{
    int function;
    int type_id;
    int inversion;
    int duration;
    struct harmony* next;
} Harmony;

typedef struct
{
    FILE* fp;
    int chunk_size;
    int subchunk1_size;
    int audio_format;
    int num_channels;
    int sample_rate;
    int byte_rate;
    int block_align;
    int bits_per_sample;
    int subchunk2_size;
} wavFileInfo;


extern int global_seed;

// Tyler's functions:
Part* read(char* wavfile, int bpm, int divspermeasure);
int findAvgs(wavFileInfo* info, double avg[], int num_avg);
int findClumps(gsl_histogram* h, int max_key);
int openWavFile(wavFileInfo* info);
int makeWindow(wavFileInfo* info, double* data_left, double* data_right, int note_length);
int analyzeData(double* data, FILE* out, wavFileInfo* info, int current_size);
double* diff(double data[], int n);
double max(double data[], int n);
int powerOfTwo(int v);


// Phil's functions

/**
*   Intoduce an offset for a pickup measure
**/
Part* addPickup(Part* part, int beats, int meter);

/**
*   Follow these guidelines when choosing a chord
**/
int allowedChordProgression(int first, int second);

/**
*   Returns negative one if the note should be flat, positive one if the note should be sharp
**/
int alterOf(char* note, int key);

/**
*   Return a Rhythm with the same rhythm as a part
**/
Rhythm* copyPartRhythm(Part* melody);

/**
*   Create a totally random input file
*       1) key is somewhere between -7 and 7 inclusive
*       2) chord progression starts on 1 and ends on 1
*       3) length is somewhere bewteen 5 and 40 sections
**/
int createRandomInput(char* filename);

/**
*   Returns a pointer to an array of scale degree chords that contain the note.
**/
int* determineHarmonicScaleDegrees(Part* part, int key);

/**
*   Returns a pointer to the first node of a Harmony
*   Rhythm must be longer than the Part (in divisions, but not necessarily nodes)
**/
Harmony* determineHarmony(Part* part_head, Rhythm* rhythm_head, int key, int beats);

/**
*   Determine the meter given a part 
**/
int* determineMeter(Part* part);

/**
*   Generates a random arpeggio pattern
**/
int generateArpeggioPattern(int pattern[], int length);

/**
*   Generates a random rhythm with the number of notes specified and over the duration specified
**/
int generateRhythmPattern(int rhythm[], int num_notes, int duration);

/**
*   Writes a counterpoint part given a harmony and some other parts
**/
Part* getCounterpointPart(Harmony* harmony, Rhythm* rhythm, Part* other_parts[], int num_parts, int key, int staff);

/**
*   Given a key number, returns a note struct with the following values:
*       .octave is the octave (octave 4 starts at middle C) if the key #
*       .alter is sharp = 1, natural = 0, flat = -1 depends on the provided key signature
*       .step is the alphabetical note name A = 0, B = 1 ...
*       .duration is the duration in whatever units are provided (quarter note, usually)
*   (see http://en.wikipedia.org/wiki/Piano_key_frequencies)
**/
Note getNote(int piano_key, int key_sig);

/**
*   Fills the notes and alters arrays according to the properties supplied in section/key
**/
int getNotes(int notes[], int alters[], Section section, int key);

/** 
*   Imports a melody from a text file in a csv with two columns:
*      1) key number (1-88)
*      2) duration
**/
Part* getPart(const char* filename, int key);

/**
*   Returns a range of notes that are allowed
**/
Range getRange(int function, int type_id, int key);

/**
*   Returns a Rhythm with the specifications given
**/
Rhythm* getRhythm(int divisions, int beats, int style);

/**
*   Returns a struct with information about the type
*      0: Major
*      1: Minor
*      2: Diminished
*      3: Augmented
**/
Type getType(int id);

/**
*   Returns a struct with compostion attributes supplied by a user
*     -If the filename supplied is invalid, it prompts user for input from the command line
**/
Composition getUserInput(char* filename);

/**
*   Returns 1 if the note is in range, 0 if not
**/
int isInRange(Range range, int note_num);

/**
*   Cleanup functions - free these linked lists.
**/
int rmHarmony(Harmony* head);
int rmRhythm(Rhythm* head);
int rmPart(Part* head);

/**
*   Transposes the melody, returns the new key
**/
int transpose(Part* part, int old_key, int new_key, int shift_direction);

/**
*   Writes an arpeggio in the measure provided
**/
int writeArpeggio(xmlNodePtr measure, Section section, int key, float note_dur);

/**
*   Writes a chord (should be the first in the sequence - no backup tag
**/
int writeChord(xmlNodePtr measure, Section section, int key);

/**
*   Writes a harmony to a file
**/
int writeHarmony(const char* filename, Harmony* harmony_head, int key);

/**
*   Writes an xml header appropriate for a music xml file
**/
xmlNodePtr writeHeader(xmlDocPtr doc, char* composer, char* title);

/**
*   Writes measure attributes to xml
**/
int writeMeasureAttributes(xmlNodePtr measure, MeasureAttribute measure_attributes);

/**
*   Writes a melody in the measure provided
**/
int writeMelody(xmlNodePtr measure, Section sect, int key, float num);

/**
*   Writes the music according to the specifications of the user
**/
int writeMusic(const char* filename, Composition composition);

/**
*   Writes a single note according to specification within the measure supplied
**/
int writeNote(xmlNodePtr measure, Note note, int num_divs, int tie_type,
        int beam_pos, int chord, int staff, int numeral, int type, int rest);

/**
*   Writes a part to a file
**/
int writePart(const char* filename, Part* part[], int num_parts, int beat, int key, char* composer, char* title);

/**
*   Writes a section (which may not be exactly a measure long) in the part provided
**/
xmlNodePtr writeSection(xmlNodePtr part, xmlNodePtr last_measure, Section section, int key);

#endif
