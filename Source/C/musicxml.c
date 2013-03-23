/********************************************************************************
 *
 * Music XML Library
 *
 *  © Phil Ngo, Tyler Clites 2012
 *  
 *  CS50 Final Project
 *
 * This library contains functions specifically designed for 3 purposes:
 *      1) Analyzing and parsing .wav files for frequency content, and storing
 *         this information in a compact way. 
 *      2) to harmonize arbitrary melodies and write parts according to those
 *         harmonizations.
 *      3) to help notate music in readable musicxml according to the 
 *         specifications provided by makemusic©. These xml files are readable 
 *         in Finale©, Sibelius© and most major music notation programs.
 * 
 * It also contains a number of functions devoted to parsing .wav files for 
 *  frequency content, and for storing that information compactly for later use.
 *
********************************************************************************/ 

#include "musicxml.h"

// used to randomize using system time.
int global_seed = 1;

/**
*   Intoduce an offset for a pickup measure
**/
Part* addPickup(Part* part, int beats, int meter)
{
    if (beats > 0)
    {
        // add a rest of the appropriate duration to the beginning of the Part
        Part* pickup = malloc(sizeof(Part));
        pickup->note_num = part->note_num;
        pickup->duration = (meter - beats) * DIVISIONS;
        pickup->staff = part->staff;
        pickup->rest = 1;
        pickup->next = part;
        return pickup;
    }
    else
        return part;
}

/**
*   Follow these guidelines when choosing a chord
*   Returns 1 if allowed, 0 if not allowed.
*   arguments: takes two numbers from 0 to 6 inclusive
**/
int allowedChordProgression(int first, int second)
{
    // this matrix stores information about which chords can follow a given chord
    int allowed_matrix[7][7] = {{1,1,1,1,1,1,1},
                                {0,1,0,0,1,0,1},
                                {0,1,1,1,0,1,0},
                                {1,0,0,1,1,0,1},
                                {1,0,0,0,1,1,0},
                                {0,1,0,1,0,1,0},
                                {1,0,0,0,0,0,1}};

    return allowed_matrix[first][second];
}

/**
* Return values; -1 = flat, 0 = natural, 1 = sharp
**/
int alterOf(char* note, int key)
{
    char* sharps[7] = {"F","C","G","D","A","E","B"};
    char* flats[7] = {"B","E","A","D","G","C","F"};
    
    for (int i = 0, n = fabs(key); i < n; i++)
    {
        if (key > 0 && strcmp(note, sharps[i]) == 0)
            return 1;
        else if (key < 0 && strcmp(note, flats[i]) == 0)
            return -1;
    }
    return 0;
}

/**
*   Returns a rhythm with the same rhythm as the part
**/
Rhythm* copyPartRhythm(Part* part)
{
    // error checking
    if (part == NULL)
    {
        printf("Error: Bad Part\n");
        return NULL;
    }

    // declare a new head pointer
    Rhythm* rhythm = malloc(sizeof(Rhythm));

    // loop over part, copying the durations
    Part* part_ptr = part;
    Rhythm* rhythm_ptr = rhythm;
    while (part_ptr != NULL)
    {
        rhythm_ptr->divisions = part_ptr->duration;
        part_ptr = part_ptr->next;
        if (part_ptr != NULL)
        {
            rhythm_ptr->next = malloc(sizeof(Rhythm));
            rhythm_ptr = rhythm_ptr->next; 
        }
        else
            rhythm_ptr->next = NULL;
    }

    // return the head pointer
    return rhythm;
}

/**
*   Create a totally random input file
*       1) key is somewhere between -7 and 7 inclusive
*       2) chord progression starts on 1 and ends on 1
*       3) length is somewhere bewteen 5 and 40 sections
**/
int createRandomInput(char* filename)
{
    // reseed to psuedo-randomize
    srand(time(NULL) * global_seed++);
    
    // try to open file to write to
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error: Bad filename in createRandomInput\n");
        return 1;
    }

    // random key and length, random number of chords per measure
    int key = (rand() % 15) - 7;
    int num_measures = (rand() % 11) + 5;
    int changes_per_measure = 3;

    // preallocate arrays for scale degree, type and durations
    int scale_degree[num_measures * changes_per_measure];
    int type[num_measures * changes_per_measure];

    // write key and length to the file
    fprintf(fp, "%d\n%d\n", key, num_measures * changes_per_measure);
    
    // write each chord change
    for (int i = 0; i < num_measures * changes_per_measure; i++)
    {
        scale_degree[i] = (i % 7) + 1;
        type[i] = i % 4;
        fprintf(fp, "%d,%d,%d\n", scale_degree[i], type[i], (i % changes_per_measure == changes_per_measure - 1)? 2: 1);
    }
    
    fclose(fp);
    return 0;
}

/**
*   Given a part, this function returns a pointer to an array of length seven with acceptable scale degrees.
**/
int* determineHarmonicScaleDegrees(Part* part, int key)
{
    if (part == NULL || key < -7 || key > 7)
    {
        printf("Error determining scale degrees - bad part or key\n");
        return NULL;
    }
    
    // key runs from -7 to 7. add seven to make this index an array.
    //                   0  1  2   3  4  5  6  7  8  9   10 11 12 13 14
    //                   Cb Gb Db  Ab Eb Bb F  C  G  D   A  E  B  F# C#
    int norm_to_C[15] = {1, 6, 11, 4, 9, 2, 7, 0, 5, 10, 3, 8, 1, 6, 11};
    
    // major scale degrees. non-chord tones are labeled 0;
    int scale_degrees[12] = {1, 0, 2, 0, 3, 4, 0, 5, 0, 6, 0, 7};
    
    // scale degree of the note in the part
    int scale_degree = scale_degrees[(part->note_num + norm_to_C[key] + 7) % 12];
    
    // an array of diatonic chords that contain the given scale degree
    int* possible_degrees = malloc(sizeof(int) * 3);
    possible_degrees[0] = scale_degree;
    possible_degrees[1] = scale_degree == 0 ? 0 : ((scale_degree + 2) % 7) + 1;
    possible_degrees[2] = scale_degree == 0 ? 0 : ((scale_degree + 4) % 7) + 1;

    return possible_degrees;
}

/**
*   Figure out a harmony pattern given a melody
**/
Harmony* determineHarmony(Part* part_head, Rhythm* rhythm_head, int key, int beats)
{
    // error checking
    if (part_head == NULL || rhythm_head == NULL)
    {
        printf("Error determining harmony: no part or no rhythm\n");
        return NULL;
    }

    // randomize
    srand(time(NULL) * global_seed++);

    // keep track of the part, rhythm and harmony head pointers
    Part* part_ptr = part_head;
    Rhythm* rhythm_ptr = rhythm_head;
    Harmony* harmony_head = NULL;
    Harmony* harmony_ptr;

    // determine length of part
    int part_node_count = 0;
    int part_divisions = 0;
    while (part_ptr != NULL)
    {
        part_node_count++;
        part_divisions += part_ptr->duration;
        part_ptr = part_ptr->next;
    }

    // determine length of rhythm
    int rhythm_node_count = 0;
    int rhythm_divisions = 0;
    while (rhythm_ptr != NULL)
    {
        rhythm_node_count++;
        rhythm_divisions += rhythm_ptr->divisions;
        rhythm_ptr = rhythm_ptr->next;
    }

    // verify the rhythm is of appropriate length
    if (rhythm_divisions < part_divisions)
    {
        printf("Error: Rhythm is not as long as the part\n");
        return NULL;
    }

    // declare an array to hold all of the possible harmonies
    int possible_harmonies[part_node_count][4];

    // declare an array to keep track of the downbeat
    int downbeat_factor[part_node_count];

    // loop over the part, filling possible_harmonies with chords 
    // that contain the note in the part, grouping by the grouping rhythm
    part_ptr = part_head;
    rhythm_ptr = rhythm_head;
    int rhythm_node_num = 0;
    int div_counter = 0;
    int total_div_counter = 0;
    for (int i = 0; i < part_node_count; i++)
    {
        // figure out if were on a downbeat and apply a weight to notes on a downbeat
        if (beats == 4)
        {
            if (total_div_counter % (DIVISIONS * beats) == 0)
                downbeat_factor[i] = 4;
            else if (total_div_counter % (DIVISIONS * beats / 2) == 0)
                downbeat_factor[i] = 3;
            else
                downbeat_factor[i] = 2;
        }
        else if (beats == 3)
        {
            if (total_div_counter % (DIVISIONS * beats) == 0)
                downbeat_factor[i] = 2;
            else
                downbeat_factor[i] = 1;
        }
        else
        {
            printf("Error (determineHarmony): # beats not supported\n");
            return NULL;
        }

        // populate the array with harmonies
        int* temp = determineHarmonicScaleDegrees(part_ptr, key);
        possible_harmonies[i][0] = temp[0];
        possible_harmonies[i][1] = temp[1];
        possible_harmonies[i][2] = temp[2];
        free(temp);

        // index which part of the harmony rhythm we're in (index starts at 0)
        possible_harmonies[i][3] = rhythm_node_num;
        div_counter += part_ptr->duration;
        total_div_counter += part_ptr->duration;
        if (div_counter >= rhythm_ptr->divisions)
        {
            div_counter = 0;
            rhythm_node_num++;
            rhythm_ptr = rhythm_ptr->next;
        }

        // next node
        part_ptr = part_ptr->next;
    }
    
    // declare an array that puts a harmony to each segment of a length defined by the Rhythm
    // this will contain a number 1-7 for each node in rhythm, and will be synthesized into a harmony
    int harmony[rhythm_node_count];

    // populate some helper arrays with more information about possible chord structures
    int bins[rhythm_node_count][7];
    int bin_max[rhythm_node_count];
    int node = 0;
    for (int i = 0; i < rhythm_node_count; i++)
    {
        // initialize to 0
        for (int j = 0; j < 7; j++)
            bins[i][j] = 0;

        // sum the possible chords for this measure
        while (possible_harmonies[node][3] == i)
        {
            for (int j = 0; j < 3; j++)
                if (possible_harmonies[node][j] != 0)
                    bins[i][possible_harmonies[node][j] - 1] += downbeat_factor[node];
                node++;

            //don't inadvertently access out of bounds
            if (node >= part_node_count)
                break;
        }

        // find the bin maximum
        bin_max[i] = 0;
        for (int j = 0; j < 7; j++)
            if (bins[i][j] > bin_max[i])
                bin_max[i] = bins[i][j];

    }

    // loop over progressions, trying to find one that works best (how do we score these)
    int good_progression = 1;
    int iteration = 0;
    int max_iterations = 0;
    do
    {
        // loop over harmonic rhythm nodes, determining for each which harmony should be used
        for (int i = 0; i < rhythm_node_count; i++)
        {
            // default to tonic
            harmony[i] = 1;
            if (i == 0 || i == rhythm_node_count - 1)
                continue;
            else
            {
                int rand_index;
                do
                {
                    rand_index = rand() % 7;
                    harmony[i] = rand_index + 1;
                }
                while (bins[i][rand_index] != bin_max[i]);
            }
        } 

        // check to see if this harmony works
        good_progression = 1;
        for (int j = 1; j < rhythm_node_count; j++)
        {
            if (!allowedChordProgression(harmony[j - 1], harmony[j]))
                good_progression = 0;
        }
        iteration++;
    }
    while (!good_progression && iteration <= max_iterations);

    // diatonic types:
    int type_of[7] = {0, 1, 1, 0, 0, 1, 2};

    // loop over parts, writing a harmony
    part_ptr = part_head;
    rhythm_ptr = rhythm_head;
    int current_node = 0;
    while (rhythm_ptr != NULL)
    {
        // initialize the new harmony
        if (harmony_head == NULL)
        {
            harmony_head = malloc(sizeof(Harmony));
            harmony_ptr = harmony_head;
        }

        // fill with information
        harmony_ptr->function = harmony[current_node++];
        harmony_ptr->type_id = type_of[harmony_ptr->function - 1];
        harmony_ptr->inversion = 0;
        harmony_ptr->duration = rhythm_ptr->divisions;

        // only allocate memory if necessary
        if (rhythm_ptr->next != NULL)
            harmony_ptr->next = malloc(sizeof(Harmony));
        else 
            harmony_ptr->next = NULL;

        // go to the next node
        rhythm_ptr = rhythm_ptr->next;
        harmony_ptr = harmony_ptr->next;
    }

    // return a reference to the first node of the Harmony
    return harmony_head;
}

/**
*   determine the meter of a given part
**/
int* determineMeter(Part* part)
{
    int meter[2] = {288, 384};
    int max_score = 0;
    int final_offset = 0;
    int final_meter = 0;

    for (int offset = 0; offset < 384; offset += DIVISIONS)
    {
        Part* ptr = part;
        int meter_score[2] = {0,0};
        int div_counter = 0;
        
        while (ptr != NULL)
        {
            for (int i = 0; i < 2; i++)
            {
                if (div_counter % meter[i] == offset)
                    meter_score[i] += 2;
                else if (i == 1 && div_counter % meter[i] == (meter[i] / 2) + offset)
                    meter_score[i]++;
            }
            div_counter += ptr->duration;
            ptr = ptr->next;
        }
        
        for (int i = 0; i < 2; i++)
        {
            if (max_score < meter_score[i])
            {
                max_score = meter_score[i];
                final_offset = offset;
                final_meter = (i == 0) ? 3 : 4;
            }
        }
    }
    
    if (final_meter == 0)
        return NULL;

    int* final_values = malloc(sizeof(int) * 2);
    final_values[0] = final_meter;
    final_values[1] = final_offset;
    
    return final_values;
}

/**
* Generates a random arpeggio pattern
**/
int generateArpeggioPattern(int pattern[], int length)
{
    // pattern starting place
    srand(time(NULL) * global_seed);
    global_seed++;

    int place = rand() % 4;

    // increment pattern up or down by one randomly
    for (int i = 0; i < length; i++)
    {
        int r = rand() % 3;
        switch (r)
        {    
            case 0:
                place--;
                break;
            case 1:
                break;
            case 2:
                place++;
                break;
        }                   
        pattern[i] = (place >= 0) ? place : 0;
    }

    return 0;
}

/**
* Generates a random rhythm with the number of notes specified and over the duration specified
**/
int generateRhythmPattern(int rhythm[], int num_notes, int duration)
{
    int num_divisions = DIVISIONS * duration;
    rhythm[0] = num_divisions;
    
    // seed
    srand(time(NULL) * global_seed++);

    // keep dividing notes in half
    for (int i = 1; i < num_notes; i++)
    {
        int r = rand() % i;
        rhythm[r] /= 2;
        rhythm[i] = rhythm[r];
    }

    return 0;
}

/**
*   Writes a counterpoint part given a harmony and a rhythm
**/
Part* getCounterpointPart(Harmony* harmony, Rhythm* rhythm, Part* other_parts[], int num_parts, int key, int staff)
{

    // these are the acceptable ranges to make bass and tenor parts - key 25 to key 40
    int upper_bound = 40;
    int lower_bound = 25;
    
    // allocate memory for a head node of a part
    Part* new_part_head = malloc(sizeof(Part));
    
    // randomize
    srand(time(NULL) * global_seed++);
    
    // useful for keeping track of where we are if harmony and parts are timed differently
    int harmony_divisions = 0;
    int rhythm_divisions = 0;
    int part_divisions[num_parts];
    for (int i = 0; i < num_parts; i++)
        part_divisions[i] = 0;
   
    // key runs from -7 to 7. add 7 to make this index an array. ex) norm_from_C[key + 7]
    //                     0   1  2  3  4  5   6  7  8  9  10 11 12  13 14
    //                     Cb  Gb Db Ab Eb Bb  F  C  G  D  A  E  B   F# C#
    int norm_from_C[15] = {11, 6, 1, 8, 3, 10, 5, 0, 7, 2, 9, 4, 11, 6, 1};

    // start in the octave below middle C (C3 to C4)
    int reference_note = 28 + norm_from_C[key + 7];

    // some pointers to keep track of the place in the rhythms and melodies.
    Rhythm* rhythm_ptr = rhythm;
    Part* new_part_ptr = new_part_head;
    Part* prev_part_ptr = new_part_head;
    Harmony* harmony_ptr = harmony;
    Part* other_part_ptr[num_parts];
    for (int i = 0; i < num_parts; i++)
        other_part_ptr[i] = other_parts[i];

    // track the previous note
    int previous_note = reference_note;

    // loop over nodes in the rhythm
    while (rhythm_ptr != NULL && harmony_ptr != NULL)
    {
        // determine which notes are allowed
        Range allowed_range = getRange(harmony_ptr->function, harmony_ptr->type_id, key);

        // loop over the notes in the range and eliminate the ones that aren't in range
        for (int i = 0; i < 88; i++)
            if (i + 1 > upper_bound || i + 1 < lower_bound)
                allowed_range.notes[i] = 0;
        
        // disallow notes that are used in other parts
        for (int i = 0; i < num_parts; i++)
            if (other_part_ptr[i] != NULL)
                allowed_range.notes[other_part_ptr[i]->note_num - 1] = 0;

        // pick a note that's allowed within the rules of counterpoint
        new_part_ptr->note_num = previous_note;
        for (int i = 1; i < 12; i++)
        {
            if (isInRange(allowed_range, previous_note + i) && isInRange(allowed_range, previous_note - i))
            {
                if (rand() % 2 == 0)
                {
                    new_part_ptr->note_num = previous_note + i;
                    break;
                }
                else
                {
                    new_part_ptr->note_num = previous_note - i;
                    break;
                }
            }
            else if (isInRange(allowed_range, previous_note + i))
            {
                new_part_ptr->note_num = previous_note + i;
                break;
            }
            else if (isInRange(allowed_range, previous_note - i))
            {
                new_part_ptr->note_num = previous_note - i;
                break;
            }
        }
        new_part_ptr->duration = rhythm_ptr->divisions;
        new_part_ptr->staff = staff;
        new_part_ptr->rest = 0;
        new_part_ptr->next = malloc(sizeof(Part));

        // remember the previous note
        previous_note = new_part_ptr->note_num;

        // update the current harmony
        rhythm_divisions += rhythm_ptr->divisions;
        if (harmony_divisions < rhythm_divisions)
        {
            harmony_divisions += harmony_ptr->duration;
            harmony_ptr = harmony_ptr->next;
        }

        // update each of the parts
        for (int i = 0; i < num_parts; i++)
        {
            if (part_divisions[i] < rhythm_divisions && other_part_ptr[i] != NULL)
            {
                part_divisions[i] += other_part_ptr[i]->duration;
                other_part_ptr[i] = other_part_ptr[i]->next;
            }
        }

        // next
        rhythm_ptr = rhythm_ptr->next;
        prev_part_ptr = new_part_ptr;
        new_part_ptr = new_part_ptr->next;
    }
    
    // free heap memory!
    if (new_part_ptr == new_part_head)
        new_part_head = NULL;
    free(prev_part_ptr->next);
    prev_part_ptr->next = NULL;

    return new_part_head;
}

/**
*   Given a key number, returns a note struct with the following values:
*       .octave is the octave (octave 4 starts at middle C) if the key #
*       .alter is sharp = 1, natural = 0, flat = -1 depends on the provided key signature
*       .step is the alphabetical note name C = 0, D = 1 ...
*       .duration is the duration in whatever units are provided (quarter note, usually)
*   (see http://en.wikipedia.org/wiki/Piano_key_frequencies)
**/
Note getNote(int key_number, int key_sig)
{
    Note this_note;
    if (key_number < 1 || key_number > 88)
    {
        printf("Error: not an acceptable note\n");
        this_note.octave = 0;
        this_note.alter = 0;
        this_note.step = 0;
        this_note.accidental = 0;
    }
    else
    {   
        int accidentals[15][12] = { {0,2,0,0,2,0,2,0,0,2,0,2},
                                    {0,2,0,0,2,0,2,0,0,0,0,2},
                                    {0,2,0,2,0,0,2,0,2,0,0,2},
                                    {0,2,0,2,0,0,2,0,2,0,-1,0},
                                    {0,2,0,2,0,-1,0,0,2,0,-1,0},
                                    {-1,0,0,2,0,-1,0,0,2,0,-1,0},
                                    {-1,0,0,2,0,-1,0,-1,0,0,-1,0},
                                    {1,0,1,0,0,1,0,1,0,0,1,0},
                                    {1,0,1,0,0,1,0,1,0,2,0,0},
                                    {1,0,1,0,2,0,0,1,0,2,0,0},
                                    {0,0,1,0,2,0,0,1,0,2,0,2},
                                    {0,0,1,0,2,0,2,0,0,2,0,2},
                                    {0,2,0,0,2,0,2,0,0,2,0,2},
                                    {0,2,0,0,2,0,2,0,2,0,0,2},
                                    {0,2,0,2,0,0,2,0,2,0,0,2}};
        
        // These arrays hold values that correspond to white and black keys on a piano
        int sharp_alters[12] = {0,1,0,1,0,0,1,0,1,0,1,0};
        int flat_alters[12] = {0,-1,0,-1,0,0,-1,0,-1,0,-1,0};
        int sharp_steps[12] = {0,0,1,1,2,3,3,4,4,5,5,6};
        int flat_steps[12] =  {0,1,1,2,2,3,4,4,5,5,6,6};
        this_note.octave = floor((key_number + 8) / 12);
        this_note.alter = (key_sig >= 0) ? sharp_alters[(key_number + 8) % 12] : 
                                           flat_alters[(key_number + 8) % 12];
        this_note.step = (key_sig >= 0) ? sharp_steps[(key_number + 8) % 12] : 
                                          flat_steps[(key_number + 8) % 12];
        this_note.accidental = accidentals[7 + key_sig][key_number % 12];
    }
    return this_note;
}

/**
* Fills the notes and alters arrays according to the properties supplied in section/key
**/
int getNotes(int notes[], int alters[], Section section, int key)
{
    // auxilliary information for calculations based on diatonic major scale
    int sharps[12] = {0,0,1,1,2,3,3,4,4,5,5,6};
    int flats[12]  = {0,1,1,2,2,3,4,4,5,5,6,6};
    int sharp_alters[12] = {0,1,0,1,0,0,1,0,1,0,1,0};
    int flat_alters[12]  = {0,-1,0,-1,0,0,-1,0,-1,0,-1,0};
    int function_offsets[7] = {0,2,4,5,7,9,11};
    
    // calculations
    int n = 0;
    for (int i = 0; i < 12; i++)
    {
        if (section.type.notes[i] == 1)
        {
            int idx = (i + (7 * key + 12 * 5) + function_offsets[section.function - 1]) % 12;
            notes[n]  = (key > 0) ? sharps[idx] : flats[idx];
            alters[n] = (key > 0) ? sharp_alters[idx] : flat_alters[idx];
            n++;
        }
    }

    return 0;
}

/** 
*   Used for prototyping phase, but still useful for debugging.
*   Imports a melody from a csv file with two columns:
*      1) key number (1-88)
*      2) duration
*      returns a pointer to the first not of the part, implemented as a linked list
**/
Part* getPart(const char* filename, int key)
{
    // attempt to open the file 
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("ERROR: bad input file for melody\n");
        return NULL;
    }

    // allocate a pointer to the first Part node
    Part* head = malloc(sizeof(Part));
    Part* ptr = head;
    Part* prev;

    // buffer stores a line of characters
    char buffer[MAX_STRING];
    int index = 0;
    
    // loop over characters in the file
    for (int c = fgetc(fp); c != EOF; c = fgetc(fp))
    {
        if (c == '\n')
        {
            // terminate the string and reset the index
            buffer[index] = '\0';
            index = 0;

            // initialize the current node with the integer in the file
            ptr->duration = atoi(buffer);

            // next node
            ptr->next = malloc(sizeof(Part));
            prev = ptr;
            ptr = ptr->next;
        }
        else if (c == ',')
        {
            // if this is the first ptr
            if (head == NULL)
                head = ptr;

            // terminate the string and convert it to an integer
            buffer[index] = '\0';
            index = 0;
            ptr->note_num = atoi(buffer);
            ptr->staff = 1;
            ptr->rest = 0;
        }
        else
        {
            // collect chars
            buffer[index] = (char) c;
            index++;
        }
    }
    
    // free the extra allocation for the last pointer
    free(prev->next);
    prev->next = NULL;

    // return a reference to the first note in the part
    return head;
}

/**
*   Returns a range of notes that are allowed
**/
Range getRange(int function, int type_id, int key)
{ 
    // key runs from -7 to 7. add seven to the key to make this index an array.
    //                   0  1  2   3  4  5  6  7  8  9   10 11 12 13 14
    //                   Cb Gb Db  Ab Eb Bb F  C  G  D   A  E  B  F# C#
    int norm_factor[15] = {10, 3, 8, 1, 6, 11, 4, 9, 2, 7, 0, 5, 10, 3, 8};
    int diatonic_adjust[7] = {0, 10, 8, 7, 5, 3, 1};

    Range range;
    Type type = getType(type_id);

    for (int i = 0; i < 88; i++)
        range.notes[i] = type.notes[(i + norm_factor[key + 7] + diatonic_adjust[function - 1]) % 12];
    return range;
}

/**
*   Returns a Rhythm with the specifications given
**/
Rhythm* getRhythm(int divisions, int beats, int style)
{
    Rhythm* head = NULL;
    Rhythm* ptr;

    // error checking
    if (beats < 2 || beats > 4)
    {
        printf("Error: getRhythm: number of beats not supported\n");
        return NULL;
    }

    int div_counter = 0;
    while (div_counter < divisions)
    {
        if (head == NULL)
        {
            head = malloc(sizeof(Rhythm));
            ptr = head;
        }
        else if (ptr->next == NULL)
        {
            ptr->next = malloc(sizeof(Rhythm));
            ptr = ptr->next;
        }

        // depending on the harmonization style, output different rhythms
        switch (style)
        {
            case 0:
                ptr->divisions = DIVISIONS * beats;
                break;
            case 1:
                ptr->divisions = DIVISIONS;
                break;
            case 2:
                ptr->divisions = DIVISIONS * 2;
                break;
            default:
                ptr->divisions = DIVISIONS * beats;
        }
        div_counter += ptr->divisions;
        ptr->next = NULL;
    }

    return head;
}

/**
* Returns a struct with information about the type
*   0: Major
*   1: Minor
*   2: Diminished
*   3: Augmented
**/
Type getType(int id)
{
    Type type;
    type.id = id;
    for (int i = 0; i < 12; i++)
        type.notes[i] = 0;
    switch (id)
    {
        case 0:                 // major
            type.notes[0] = 1;
            type.notes[4] = 1;
            type.notes[7] = 1;
            return type;
        case 1:                 // minor
            type.notes[0] = 1;
            type.notes[3] = 1;
            type.notes[7] = 1;
            return type;
        case 2:                 // diminished
            type.notes[0] = 1;
            type.notes[3] = 1;
            type.notes[6] = 1;
            return type;
        case 3:                 // augmented
            type.notes[0] = 1;
            type.notes[4] = 1;
            type.notes[8] = 1;
            return type;
        case 4:                 // major 7th
            type.notes[0] = 1;
            type.notes[4] = 1;
            type.notes[7] = 1;
            type.notes[11] = 1;
            return type;
        case 5:                 // dom 7th
            type.notes[0] = 1;
            type.notes[4] = 1;
            type.notes[7] = 1;
            type.notes[10] = 1;
            return type;
        case 6:                 // min 7th
            type.notes[0] = 1;
            type.notes[3] = 1;
            type.notes[7] = 1;
            type.notes[10] = 1;
            return type;
        case 7:                 // half dim 7th
            type.notes[0] = 1;
            type.notes[3] = 1;
            type.notes[6] = 1;
            type.notes[10] = 1;
            return type;
        case 8:                 // dim 7th
            type.notes[0] = 1;
            type.notes[3] = 1;
            type.notes[6] = 1;
            type.notes[9] = 1;
            return type;
        case 9:                 // min maj 7th
            type.notes[0] = 1;
            type.notes[3] = 1;
            type.notes[7] = 1;
            type.notes[11] = 1;
            return type;
        case 10:                // flat major
            type.notes[11] = 1;
            type.notes[3] = 1;
            type.notes[6] = 1;
            return type;
        case 11:                // flat minor
            type.notes[11] = 1;
            type.notes[2] = 1;
            type.notes[6] = 1;
            return type;
        case 12:                // sharp major
            type.notes[1] = 1;
            type.notes[5] = 1;
            type.notes[8] = 1;
            return type;
        case 13:                // sharp minor
            type.notes[1] = 1;
            type.notes[4] = 1;
            type.notes[8] = 1;
            return type;
        default:
            type.id = -1;
            return type;
    }
}

/**
* Returns a struct with compostion attributes supplied by a user
*   If the filename supplied is invalid, it prompts user for input from the command line
**/
Composition getUserInput(char* filename)
{
    // declarations
    Composition composition;

    // read from a file if it's provided, otherwise get attributes from the command line
    FILE* fp_in = fopen(filename, "r");

    if (fp_in == NULL)
    {
        printf("Error reading composition file\n");
    }
    else
    {
        // key is the first line in the file
        fscanf(fp_in, "%d", &composition.key);

        // length is the second line in the file
        fscanf(fp_in, "%d", &composition.length);

        // initializations
        int type_id = 0;
        
        // scan in a line (ex: 1,0,4 = tonic, major, 4 beats)
        for (int i = 0; i < composition.length; i++)
        {
            fscanf(fp_in, "%d,%d,%f", &composition.section[i].function, &type_id, &composition.section[i].duration);
            composition.section[i].type = getType(type_id);
        }

        // close the input file
        fclose(fp_in);
        
        return composition;
    }

    // ask for length of chord sequence
    printf("How many chord changes in your piece? ");
    scanf("%d", &(composition.length)); 
    
    // ask user for sections in the sequence
    for (int i = 0; i < composition.length; i++)
    {
        // get the classical chord function
        do
        {
            printf("Input Function #%d: ", i + 1);
            scanf("%d", &(composition.section[i].function));
        }
        while (composition.section[i].function < 1 || composition.section[i].function > 7);

        // get the chord type i.e. major, minor, etc
        do
        {
            int type_id;
            printf("Input Type #%d: ", i + 1);
            scanf("%d", &type_id);
            composition.section[i].type = getType(type_id);
        }
        while (composition.section[i].type.id == -1);

        // get the chord duration
        do
        {
            printf("Input duration #%d (unit: quarter note): ", i + 1);
            scanf("%f", &(composition.section[i].duration));
        }
        while (composition.section[i].duration < .125 || composition.section[i].duration > 4);
    }

    // ask user for key (uses circle of fifths: ex) f = -1, c = 0, g = 1)
    do
    {
        printf("What key is your piece in? ");
        scanf("%d", &composition.key);
    }
    while ( -7 > composition.key || composition.key > 7);

    // return results
    return composition;
}


/**
*   Returns 1 if the note is in range, 0 if not
**/
int isInRange(Range range, int note_num)
{
    return range.notes[note_num - 1];
}

/**
*   Free memory allocated for the Harmony
**/
int rmHarmony(Harmony* head)
{
    Harmony* ptr = head;
    Harmony* old;

    // loop over Harmony nodes, freeing them
    while (ptr != NULL)
    {
        old = ptr;
        ptr = ptr->next;
        free(old);
    }
    return 0;
}

/**
*   Free memory allocated for the Part
**/
int rmPart(Part* head)
{
    Part* ptr = head;
    Part* old;

    // loop over Part nodes, freeing them
    while (ptr != NULL)
    {
        old = ptr;
        ptr = ptr->next;
        free(old);
    }
    return 0;
}

/**
*   Free memory allocated for a Rhythm linked list
**/
int rmRhythm(Rhythm* head)
{
    Rhythm* ptr = head;
    Rhythm* old;

    // loop over Harmony nodes, freeing them
    while (ptr != NULL)
    {
        old = ptr;
        ptr = ptr->next;
        free(old);
    }
    return 0;
}

/**
*   Transpose old key to new key shifting either up or down.
**/
int transpose(Part* part, int old_key, int new_key, int shift_direction)
{
    //                                F C G     
    int norm_key[15] = {11,6,1,8,3,10,5,0,7,2,9,4,11,6,1};

    if (new_key == old_key)
        return 0;
    
    int shift = norm_key[new_key + 7] - norm_key[old_key + 7];

    // determine amount to shift by
    if (shift_direction == 1 && shift < 0)
        shift += 12;
    else if (shift_direction == -1 && shift > 0)
        shift -= 12;
    else if (shift_direction != 1 & shift_direction != -1)
        printf("Error: shift direction must be either 1 or -1\n");

    // pointer to the current node
    Part* ptr = part;

    // loop over all of the parts
    while (ptr != NULL)
    {
        if (shift_direction == 1)
            if (ptr->note_num + shift > 88)
                ptr->note_num += shift - 12;
            else
                ptr->note_num += shift;
        else
            if (ptr->note_num + shift < 1)
                ptr->note_num += 12 + shift;
            else
                ptr->note_num += shift;
        ptr = ptr->next;
    }
    
    return shift;
}

/**
* Writes an arpeggio in the measure provided
**/
int writeArpeggio(xmlNodePtr measure, Section section, int key, float note_dur)
{
    // determine number of notes in the chord
    int num_notes = 0;
    for (int i = 0; i < 12; i++)
        num_notes += section.type.notes[i];

    // determine the note names and accidentals
    int note_idx[num_notes];
    int note_alter[num_notes];
    getNotes(note_idx, note_alter, section, key);

    //assume that the bass was written first and adjust the counter
    char measure_length[MAX_STRING];
    sprintf(measure_length, "%d", (int) (DIVISIONS * section.duration));
    xmlNodePtr backup = xmlNewChild(measure, NULL, BAD_CAST "backup", NULL);
    xmlNewChild(backup, NULL, BAD_CAST "duration", BAD_CAST measure_length);

    // generate a new pattern
    int num = section.duration / note_dur;
    int pattern[num];
    generateArpeggioPattern(pattern, num);

    // loop over notes in the measure
    for (int i = 0; i < num; i++)
    {
        Note note_pitch;
        note_pitch.octave = 4;
        note_pitch.alter = note_alter[pattern[i] % num_notes];
        note_pitch.step = note_idx[pattern[i] % num_notes];
        note_pitch.accidental = 0;
        writeNote(measure, note_pitch, DIVISIONS * note_dur, 0, -1, 0, 1, -1, section.type.id, 0);
    }

    return 0;
}

/**
* Writes a chord (should be the first in the sequence - no backup tag
**/
int writeChord(xmlNodePtr measure, Section section, int key)
{
    // determine number of notes in the chord
    int num_notes = 0;
    for (int i = 0; i < 12; i++)
        num_notes += section.type.notes[i];

    // determine the note names and accidentals
    int note_idx[num_notes];
    int note_alter[num_notes];
    getNotes(note_idx, note_alter, section, key);

    for (int i = 0; i < num_notes; i++)
    {
        Note note_pitch;
        note_pitch.octave = 3;
        note_pitch.alter = note_alter[i];
        note_pitch.step = note_idx[i];
        note_pitch.accidental = 0;
        writeNote(measure, note_pitch, DIVISIONS * section.duration, 0, -1, 
                (i == 0) ? 0 : 1, 2, (i == 0) ? section.function - 1 : -1, section.type.id, 0);
    }

    return 0;
}

/**
*   Writes a harmony to a file
**/
int writeHarmony(const char* filename, Harmony* harmony_head, int key)
{
    // write header, doc, root, and dtd
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr part = writeHeader(doc, "Phil", "My Harmonized Melody");
    xmlNodePtr last_measure = NULL;

    // loop over sections, writing each as a certain type
    Harmony* harmony_ptr = harmony_head;
    while (harmony_ptr != NULL)
    {
        Section section = {.function = harmony_ptr->function, 
            .type = getType(harmony_ptr->type_id), 
            .duration = harmony_ptr->duration / (float) DIVISIONS};
        last_measure = writeSection(part, last_measure, section, key);
        harmony_ptr = harmony_ptr->next;
    }

    // save the file with format information
    xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return 0;
}

/**
* Writes an xml header
**/
xmlNodePtr writeHeader(xmlDocPtr doc, char* composer, char* title)
{
    // declare relevant nodes and dtd
    xmlNodePtr root_node = NULL, partlist = NULL, scorepart = NULL, part = NULL;
    xmlDtdPtr dtd = NULL;
    
    // set root node
    root_node = xmlNewNode(NULL, BAD_CAST "score-partwise");
    xmlNewProp(root_node, BAD_CAST "version", BAD_CAST "3.0");
    xmlDocSetRootElement(doc, root_node);
    dtd = xmlCreateIntSubset(doc, BAD_CAST "score-partwise", 
             BAD_CAST  "-//Recordare//DTD MusicXML 3.0 Partwise//EN", 
             BAD_CAST  "http://www.musicxml.org/dtds/partwise.dtd");

    // header: title of the composition
    xmlNodePtr work = xmlNewChild(root_node, NULL, BAD_CAST "work", NULL);
    xmlNewChild(work, NULL, BAD_CAST "work-title", BAD_CAST title);
    
    // header: composer
    xmlNodePtr identification = xmlNewChild(root_node, NULL, BAD_CAST "identification", NULL);
    xmlNodePtr creator = xmlNewChild(identification, NULL, BAD_CAST "creator", BAD_CAST composer);
    xmlNewProp(creator, BAD_CAST "type", BAD_CAST "composer");

    // header: part-list
    partlist = xmlNewChild(root_node, NULL, BAD_CAST "part-list", NULL);
    scorepart = xmlNewChild(partlist, NULL, BAD_CAST "score-part", NULL);
    xmlNewProp(scorepart, BAD_CAST "id", BAD_CAST "P1");
    xmlNewChild(scorepart, NULL, BAD_CAST "part-name", BAD_CAST "1");
    
    // part 1
    part = xmlNewChild(root_node, NULL, BAD_CAST "part", NULL);
    xmlNewProp(part, BAD_CAST "id", BAD_CAST "P1");

    return part;
}

/**
* Writes measure attributes
**/
int writeMeasureAttributes(xmlNodePtr measure, MeasureAttribute measure_attributes)
{

    // define measure number
    char measure_num[MAX_STRING];
    sprintf(measure_num, "%d", measure_attributes.num);
    xmlNewProp(measure, BAD_CAST "number", BAD_CAST measure_num);

    // define attributes
    char divisions[MAX_STRING];
    sprintf(divisions, "%d", DIVISIONS);
    xmlNodePtr attributes = xmlNewChild(measure, NULL, BAD_CAST "attributes", NULL);
    xmlNewChild(attributes, NULL, BAD_CAST "divisions", BAD_CAST divisions);
    
    // if this is the first measure, some extra information is needed to specify key, time signature
    if (measure_attributes.num == 1)
    {
        // define key
        char key_s[MAX_STRING];
        sprintf(key_s, "%d", measure_attributes.key);
        xmlNodePtr k = xmlNewChild(attributes, NULL, BAD_CAST "key", NULL);
        xmlNewChild(k, NULL, BAD_CAST "fifths", BAD_CAST key_s);

        // define time signature
        xmlNodePtr time = xmlNewChild(attributes, NULL, BAD_CAST "time", NULL);
        if (measure_attributes.beats == 4 && measure_attributes.beat_type == 4)
            xmlNewProp(time, BAD_CAST "symbol", BAD_CAST "common");
        char beats_s[MAX_STRING];
        char beat_type_s[MAX_STRING];
        sprintf(beats_s, "%d", measure_attributes.beats);
        sprintf(beat_type_s, "%d", measure_attributes.beat_type);
        xmlNewChild(time, NULL, BAD_CAST "beats", BAD_CAST beats_s);
        xmlNewChild(time, NULL, BAD_CAST "beat-type", BAD_CAST beat_type_s);
    }
    
    // define number of staves - default 2
    xmlNewChild(attributes, NULL, BAD_CAST "staves", BAD_CAST "2");

    if (measure_attributes.num == 1)
    {
        // define clefs
        xmlNodePtr g_clef = xmlNewChild(attributes, NULL, BAD_CAST "clef", NULL);
        xmlNewProp(g_clef, BAD_CAST "number", BAD_CAST "1");
        xmlNewChild(g_clef, NULL, BAD_CAST "sign", BAD_CAST "G");
        xmlNewChild(g_clef, NULL, BAD_CAST "line", BAD_CAST "2");
        xmlNodePtr f_clef = xmlNewChild(attributes, NULL, BAD_CAST "clef", NULL);
        xmlNewProp(f_clef, BAD_CAST "number", BAD_CAST "2");
        xmlNewChild(f_clef, NULL, BAD_CAST "sign", BAD_CAST "F");
        xmlNewChild(f_clef, NULL, BAD_CAST "line", BAD_CAST "4");
    }

    return 0;
}
/**
* Writes a melody in the measure provided
**/
int writeMelody(xmlNodePtr measure, Section section, int key, float num)
{
    // determine number of notes in the chord
    int num_notes = 0;
    for (int i = 0; i < 12; i++)
        num_notes += section.type.notes[i];

    // determine the note names and accidentals
    int note_idx[num_notes];
    int note_alter[num_notes];
    getNotes(note_idx, note_alter, section, key);
    
    //assume that the bass was written first and adjust the counter
    char measure_length[MAX_STRING];
    sprintf(measure_length, "%d", (int) (DIVISIONS * section.duration));
    xmlNodePtr backup = xmlNewChild(measure, NULL, BAD_CAST "backup", NULL);
    xmlNewChild(backup, NULL, BAD_CAST "duration", BAD_CAST measure_length);

    // generate a new arpeggio pattern
    int pattern[(int) (num * section.duration)];
    generateArpeggioPattern(pattern, (int) (num * section.duration));

    // generate a new rhythm pattern
    int rhythm[(int) (num * section.duration)];
    generateRhythmPattern(rhythm, (int)(num * section.duration), section.duration);

    // loop over notes in the measure
    for (int i = 0; i < (int)(num * section.duration); i++)
    {
        Note note_pitch = {.octave = 4, 
            .alter = note_alter[pattern[i] % num_notes], 
            .step = note_idx[pattern[i] % num_notes],
            .accidental = 0};
        writeNote(measure, note_pitch, rhythm[i], 0, -1, 0, 1, -1, section.type.id, 0);
    }

    return 0;
}

/**
*   Writes the music according to the specifications of the user
**/
int writeMusic(const char* filename, Composition composition)
{
    // write header, doc, root, and dtd
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr part = writeHeader(doc, "Phil", "My Harmonized Melody");
    xmlNodePtr last_measure = NULL;

    // loop over sections, writing each as a certain type
    for (int i = 0; i < composition.length; i++)
        last_measure = writeSection(part, last_measure, composition.section[i], composition.key);

    // save the file with format information
    xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}

/**
*    Write a note according to specification within the measure supplied
**/
int writeNote(xmlNodePtr measure, Note note_pitch, int num_divs, int tie_type, 
               int beam_pos, int chord, int staff, int numeral, int type_id, int rest)
{   
    // constants
    char* numerals[10][7] = {{"I", "II", "III", "IV", "V", "VI", "VII"},
                            {"i", "ii", "iii", "iv", "v", "vi", "vii"},
                            {"i°", "ii°", "iii°", "iv°", "v°", "vi°", "vii°"},
                            {"I+", "II+", "III+", "IV+", "V+", "vi+", "vii+"},
                            {"Imaj7", "IImaj7", "IIImaj7", "IVmaj7", "Vmaj7", "VImaj7", "VIImaj7"},
                            {"I7", "II7", "III7", "IV7", "V7", "VI7", "VII7"},
                            {"i7", "ii7", "iii7", "iv7", "v7", "vi7", "vii7"},
                            {"i°7", "ii°7", "iii°7", "iv°7", "v°7", "vi°7", "vii°7"},
                            {"iø7", "iiø7", "iiiø7", "ivø7", "vø7", "viø7", "viiø7"},
                            {"I∆7", "II∆7", "III∆7", "IV∆7", "V∆7", "VI∆7", "V∆7"}};
    // start a new note
    xmlNodePtr note = xmlNewChild(measure, NULL, BAD_CAST "note", NULL);
    
    if (rest == 0)
    {

        // prepare strings
        char step_s[MAX_STRING];
        sprintf(step_s, "%c", 'A' + ( 2 + note_pitch.step) % 7);

        char octave_s[MAX_STRING];
        sprintf(octave_s, "%d", note_pitch.octave);

        char alter_s[MAX_STRING];
        sprintf(alter_s, "%d", note_pitch.alter);

        char duration_s[MAX_STRING];
        sprintf(duration_s, "%d", num_divs);

        char staff_s[MAX_STRING];
        sprintf(staff_s, "%d", staff);

        // is it a chord?
        if (chord)
            xmlNewChild(note, NULL, BAD_CAST "chord", NULL);

        // define pitch and duration
        xmlNodePtr pitch = xmlNewChild(note, NULL, BAD_CAST "pitch", NULL);
        xmlNewChild(pitch, NULL, BAD_CAST "step", BAD_CAST step_s);
        xmlNewChild(pitch, NULL, BAD_CAST "alter", BAD_CAST alter_s);
        xmlNewChild(pitch, NULL, BAD_CAST "octave", BAD_CAST octave_s);
        xmlNewChild(note, NULL, BAD_CAST "duration", BAD_CAST duration_s);

        // define tie
        if (tie_type > 0 || tie_type < 0)
        {
            xmlNodePtr tie = xmlNewChild(note, NULL, BAD_CAST "tie", NULL);
            xmlNewProp(tie, BAD_CAST "type", BAD_CAST ((tie_type > 0) ? "start" : "stop"));
        }

        // define note appearance
        switch (num_divs)
        {
            case DIVISIONS * 4:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "whole");
                break;
            case DIVISIONS * 3 / 1:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "half");
                xmlNewChild(note, NULL, BAD_CAST "dot", NULL);
                break;
            case DIVISIONS * 2:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "half");
                break;
            case DIVISIONS * 3 / 2:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "quarter");
                xmlNewChild(note, NULL, BAD_CAST "dot", NULL);
                break;
            case DIVISIONS * 1:    
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "quarter");
                break;
            case DIVISIONS * 3 / 4:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "eighth");
                xmlNewChild(note, NULL, BAD_CAST "dot", NULL);
                break;
            case DIVISIONS / 2:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "eighth");
                break;
            case DIVISIONS * 3 / 8:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "16th");
                xmlNewChild(note, NULL, BAD_CAST "dot", NULL);
                break;
            case DIVISIONS / 4:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "16th");
                break;
            case DIVISIONS * 3 / 16:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "32nd");
                xmlNewChild(note, NULL, BAD_CAST "dot", NULL);
                break;
            case DIVISIONS / 8:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "32nd");
                break;
            case DIVISIONS * 3 / 32:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "64th");
                xmlNewChild(note, NULL, BAD_CAST "dot", NULL);
                break;
            case DIVISIONS / 16:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "64th");
                break;
            case DIVISIONS / 32:
                xmlNewChild(note, NULL, BAD_CAST "type", BAD_CAST "128th");
                break;
            default:
                printf("ERROR in note appearance\n");
        }

        // display the approriate accidental
        if (note_pitch.accidental == 1)
            xmlNewChild(note, NULL, BAD_CAST "accidental", BAD_CAST "sharp");
        else if (note_pitch.accidental == -1)
            xmlNewChild(note, NULL, BAD_CAST "accidental", BAD_CAST "flat");
        else if (note_pitch.accidental == 2)
            xmlNewChild(note, NULL, BAD_CAST "accidental", BAD_CAST "natural");
        
        // staff
        xmlNewChild(note, NULL, BAD_CAST "staff", BAD_CAST staff_s);
    
        // define beams if they're needed
        if (beam_pos >= 0)
        {
            // define beam position string
            char beam_pos_s[MAX_STRING];
            if (beam_pos == 0)
                strcpy(beam_pos_s, "begin");
            else if (beam_pos == 1)
                strcpy(beam_pos_s, "end");
            else if (beam_pos == 2)
                strcpy(beam_pos_s, "continue");
            else
                printf("ERROR: invalid beam position");

            xmlNodePtr beam = xmlNewChild(note, NULL, BAD_CAST "beam", BAD_CAST beam_pos_s);
            xmlNewProp(beam, BAD_CAST "number", BAD_CAST "1");
        }
    
        // display the roman numeral
        if (numeral >= 0)
        {
            // add a lyric (used for functional chord analysis)
            xmlNodePtr lyric = xmlNewChild(note, NULL, BAD_CAST "lyric", NULL);
            xmlNewProp(lyric, BAD_CAST "placement", BAD_CAST "below");
            xmlNewChild(lyric, NULL, BAD_CAST "syllabic", BAD_CAST "single");
            xmlNewChild(lyric, NULL, BAD_CAST "text", BAD_CAST numerals[type_id][numeral]);
        }
    }
    else if (rest == 1)
    {
        char divs_s[MAX_STRING];
        sprintf(divs_s, "%d", num_divs);

        xmlNewChild(note, NULL, BAD_CAST "rest", NULL);
        xmlNewChild(note, NULL, BAD_CAST "duration", BAD_CAST divs_s);
    }

    return 0;
}

/**
*   Writes an array of parts according to the following specifications:
*       1) an output filename
*       2) an array of pointers to the first node of a part
*       3) the number of parts in the array
*       4) the number of beats in each measure
*       5) the key (C = 0, F = -1, G = 1...)
**/
int writePart(const char* filename, Part* part[], int num_parts, int beats, int key, char* composer, char* title)
{
    // write header, doc, root, and dtd
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr xml_part = writeHeader(doc, composer, title);
    xmlNodePtr measure;

    // initialize an array of pointers to parts, keep track of whether or not to display ties
    Part* ptr[num_parts];
    int tie_stat[num_parts];
    for (int i = 0; i < num_parts; i++)
    {
        ptr[i] = part[i];
        tie_stat[0] = 0;
    }
    
    // process parts in parallel - this loops once per measure
    int measure_counter = 1;
    int finished = 0;
    while (!finished)
    {
        // define measure attributes
        measure = xmlNewChild(xml_part, NULL, BAD_CAST "measure", NULL);
        MeasureAttribute attributes = {.key = key, .num = measure_counter++, .beats = beats, .beat_type = 4};
        writeMeasureAttributes(measure, attributes);
        
        // count the number of divisions written per part 
        int div_counter = 0;
        int div_max = attributes.beats * DIVISIONS;

        // loop over parts for this measure
        for (int i = 0; i < num_parts; i++)
        {
            // back by the amount written in the last part
            if (div_counter != 0)
            {
                char backup_dur_s[MAX_STRING];
                sprintf(backup_dur_s, "%d", div_counter);
                xmlNodePtr backup = xmlNewChild(measure, NULL, BAD_CAST "backup", NULL);
                xmlNewChild(backup, NULL, BAD_CAST "duration", BAD_CAST backup_dur_s);
                div_counter = 0;
            }

            // if there's another note to write in the part, write it. split it if it's too long.
            while (ptr[i] != NULL)
            {
                if (div_counter < div_max)
                {
                    div_counter += ptr[i]->duration;
                    int extra_div = div_counter - div_max;
                    
                    // make sure the note is valid
                    Note note = getNote(ptr[i]->note_num, attributes.key);
                    if (note.step == 0 && note.octave == 0 && note.alter == 0)
                    {
                        printf("Error: Invalid note number\n");
                        return -1;
                    }

                    if (extra_div <= 0)
                    {
                        writeNote(measure, note, ptr[i]->duration,
                                (tie_stat[i] == 0)? 0 : -1, -1, 0, ptr[i]->staff, -1, -1, ptr[i]->rest);
                        tie_stat[i] = 0;
                        ptr[i] = ptr[i]->next;
                    }
                    // else note is too long
                    else
                    {
                        // start a note that's tied to the next measure
                        writeNote(measure, note, ptr[i]->duration - extra_div, 
                                1, -1, 0, ptr[i]->staff, -1, -1, ptr[i]->rest);
                        ptr[i]->duration = extra_div;
                        tie_stat[i] = 1;
                        div_counter -= extra_div;
                    }
                }
                else if (div_counter == div_max)
                {
                    break; // breaks out of writing this part for this measure
                }
                else
                {
                    printf("ERROR: part was processed incorrectly\n");
                    return -1;
                }
            }
            
        }
        
        // signal for the loop to end if at the end of the parts
        finished = 1;
        for (int i = 0; i < num_parts; i++)
            if (ptr[i] != NULL)
                finished = 0;
    }
    
    // save the file with format information
    xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    // success
    return 0;
}


/*
* Writes a section (which may not be exactly a measure long) in the part provided
*/
xmlNodePtr writeSection(xmlNodePtr part, xmlNodePtr last_measure, Section section, int key)
{
    // keep track of measures
    static int measure_num = 0;
    static float cur_measure = 0.0;
    xmlNodePtr measure;

    // write only to the current measure
    if (last_measure == NULL)
    {
        measure_num++;

        // new measure pointer
        measure = xmlNewChild(part, NULL, BAD_CAST "measure", NULL);

        // define measure attributes
        MeasureAttribute attributes = {.key = key, .num = measure_num, .beats = 4, .beat_type = 4};
        writeMeasureAttributes(measure, attributes);
        
        // update the running count of the measure length
        cur_measure = section.duration;
    }
    else
    {
        // use old measure pointer
        measure = last_measure;

        // add to old duration
        cur_measure += section.duration;
    }

    // write chords
    writeChord(measure, section, key);

    // write arpeggio chord voicings
    writeArpeggio(measure, section, key, 1.0/2.0);

    if (fmod(cur_measure, 4) != 0)
        return measure;
    else
        return NULL;
}


////////////////////
////////////////////
/******************
* TYLER'S FUNCTIONS
*******************/
////////////////////
////////////////////

Part* read(char* wavfile, int bpm, int divspermeasure)
{
    // open files
    wavFileInfo* info = malloc(sizeof(wavFileInfo));
    if (info == NULL)
    {
        return NULL;
    }

    info->fp = fopen(wavfile, "r");
    if (info->fp == NULL)
    {
        printf("Error opening WAVE file.\n");
        free(info);
        return NULL;
    }
    
    if (openWavFile(info) != 0)
    {
        printf("Error reading WAVE file.\n");
        fclose(info->fp);
        free(info);
        return NULL;
    }
    
    // Compatibility Check
    if (info->bits_per_sample != 16)
    {
        printf("Error: Only 16-bit samples supported.\n");
        fclose(info->fp);
        free(info);
        return NULL;
    }
    
    FILE* out = fopen("visual.txt", "w");
    if (out == NULL)
    {
        printf("Error opening file.\n");
        fclose(info->fp);
        free(info);
        return NULL;
    }


    // determine the number of stored averages
    int num_avg = info->subchunk2_size / (AVG_WINDOW * sizeof(int16_t));
    if (info->num_channels == 2)
    {
        num_avg /= 2;
    }

    // find and store the average of each [AVG_WINDOW] points
    double avg[num_avg];
    findAvgs(info, avg, num_avg);

    // find and store the derivative of the averages
    double* differences = diff(avg, num_avg);
    if (differences == NULL)
    {
        printf("Error allocating memory.\n");
        fclose(info->fp);
        fclose(out);
        free(info);
        return NULL;
    }

    // find the maximum derivative and set threshold
    double threshold = max(differences, num_avg) * THRESHOLD_FACTOR;
    
    // create a singly linked-list of notes
    // initializations
    Part* head = NULL;
    Part* cursor = NULL;
    Part* new_part = NULL;
    int duration_total = 0;
    int counter = 0;
    int start = 0;
    int current_size = 0;
    int note_length = 0;
    double* data_left = NULL;
    double* data_right = NULL;

    // check each derivative to see if greater than threshold
    for (int i = 0; i < num_avg - 1; i++)
    {
        // if greater, add a part to the linked list
        if (abs(differences[i]) >= threshold)
        {
            // first one?
            if (head == NULL)
            {
                head = malloc(sizeof(Part));
                if (head == NULL)
                {
                    printf("Error allocating memory.\n");
                    fclose(info->fp);
                    fclose(out);
                    free(info);
                    free(differences);
                    return NULL;
                }
                
                // initialize the head
                head->note_num = 0;
                head->duration = 0;
                head->staff = 1;
                head->rest = 0;
                head->next = NULL;

                // search return to search for next note
                counter++;
                start = i * AVG_WINDOW;
                
                // skip ahead the length of a 16th note
                i += (info->sample_rate / (4 * bpm / 60)) / AVG_WINDOW - 1;
                cursor = head;
            }
            else
            {
                // edit previous node
                note_length = i * AVG_WINDOW - start;
                
                // reallocate memory to expand array if necessary
                if (note_length > current_size)
                {
                    data_left = realloc(data_left, sizeof(double) * powerOfTwo(note_length));
                    if (data_left == NULL)
                    {
                        printf("Error allocating memory for data array\n");
                        fclose(info->fp);
                        fclose(out);
                        free(info);
                        free(differences);
                        free(data_left);
                        if (info->num_channels == 2)
                        {
                            free(data_right);
                        }
                        rmPart(head);
                        return NULL;
                    }
                    current_size = powerOfTwo(note_length);
                    
                    // reallocate for second channel, if necessary
                    if (info->num_channels == 2)
                    {
                        data_right = realloc(data_right, sizeof(double) * powerOfTwo(note_length));
                        if (data_right == NULL)
                        {
                            printf("Error allocating memory for data array\n");
                            fclose(info->fp);
                            fclose(out);
                            free(info);
                            free(differences);
                            free(data_left);
                            if (info->num_channels == 2)
                            {
                                free(data_right);
                            }
                            rmPart(head);
                            return NULL;
                        }
                    }
                }
                
                
                // reset file position to the start of the note
                fseek(info->fp, 36 + (start * info->num_channels * sizeof(int16_t)), SEEK_SET);
                
                // create data array based on note length and determine note
                makeWindow(info, data_left, data_right, note_length);
                cursor->note_num = analyzeData(data_left, out, info, current_size);
                if (cursor->note_num == -1)
                {
                    printf("Error analyzing data array\n");
                    fclose(info->fp);
                    fclose(out);
                    free(info);
                    free(differences);
                    free(data_left);
                    if (info->num_channels == 2)
                    {
                        free(data_right);
                    }
                    rmPart(head);
                    return NULL;
                }
                
                
                // ensure that the segment is not noise, then begin to fill part 
                if (cursor->note_num > 0)
                {
                    // determine the duration and convert to an agreed upon standard (96 is a quarter note)
                    cursor->duration = round(((float)(bpm * (note_length)) / (info->sample_rate * 60)) * 4.0) * NOTESCALEFACTOR;
                    
                    // keep track of total length
                    duration_total += cursor->duration;

                    if (cursor->duration > 0)
                    {
                        // create a new node
                        new_part = malloc(sizeof(Part));
                        if (new_part == NULL)
                        {
                            printf("Error allocating memory for part\n");
                            fclose(info->fp);
                            fclose(out);
                            free(info);
                            free(differences);
                            free(data_left);
                            if (info->num_channels == 2)
                            {
                                free(data_right);
                            }
                            rmPart(head);
                            return NULL;
                        }
                        
                        cursor->next = new_part;


                        // initialize the new node
                        new_part->note_num = 0;
                        new_part->duration = 0;
                        new_part->staff = 1;
                        new_part->rest = 0;
                        new_part->next = NULL;

                        // move the cursor
                        counter++;
                        start = i * AVG_WINDOW;
                        i += (info->sample_rate / (4 * bpm / 60)) / AVG_WINDOW - 1;
                        cursor = new_part;
                    }
                    else
                    {
                        // reset start, jump ahead
                        start = i * AVG_WINDOW;
                        i += (info->sample_rate / (4 * bpm / 60)) / AVG_WINDOW - 1;
                    }
                }
                else
                {
                    // reset start, jump ahead
                    start = i * AVG_WINDOW;
                    i += (info->sample_rate / (4 * bpm / 60)) / AVG_WINDOW - 1;
                }
            }
        }
    }

    // assume final note ends at the end of the file
    note_length = num_avg * AVG_WINDOW - start;
    
    // reallocate if necessary
    if (note_length > current_size)
    {
        data_left = realloc(data_left, sizeof(double) * powerOfTwo(note_length));
        if (data_left == NULL)
        {
            printf("Error allocating memory for data array\n");
            fclose(info->fp);
            fclose(out);
            free(info);
            free(differences);
            free(data_left);
            if (info->num_channels == 2)
            {
                free(data_right);
            }
            rmPart(head);
            return NULL;
        }
        current_size = powerOfTwo(note_length);
        if (info->num_channels == 2)
        {
            data_right = realloc(data_right, sizeof(double) * powerOfTwo(note_length));
            if (data_right == NULL)
            {
                printf("Error allocating memory for data array\n");
                fclose(info->fp);
                fclose(out);
                free(info);
                free(differences);
                free(data_left);
                if (info->num_channels == 2)
                {
                    free(data_right);
                }
                rmPart(head);
                return NULL;
            }
        }
    }
    
    // analyze final note data
    makeWindow(info, data_left, data_right, note_length);
    cursor->note_num = analyzeData(data_left, out, info, current_size);
    if (cursor->note_num == -1)
        {
            printf("Error analyzing data array\n");
            fclose(info->fp);
            fclose(out);
            free(info);
            free(differences);
            free(data_left);
            if (info->num_channels == 2)
            {
                free(data_right);
            }
            rmPart(head);
            return NULL;
        }
    
    // if note is valid, store it and its duration
    if (cursor->note_num > 0)
    {
        cursor->duration = round(((float)(bpm * (note_length)) / (info->sample_rate * 60)) * 4.0) * NOTESCALEFACTOR;
        duration_total += cursor->duration;
        
        // cut total length at the end of a measure
        cursor->duration -= ((duration_total / NOTESCALEFACTOR) % divspermeasure) * NOTESCALEFACTOR;
    }

    // close the file
    fclose(info->fp);
    fclose(out);
    free(info);
    free(differences);
    free(data_left);
    if (info->num_channels == 2)
        free(data_right);
    
    return head;
}

int findAvgs(wavFileInfo* info, double avg[], int num_avg)
{
    // create buffer
    int16_t buffer;
    
    // initialize the avg array to zero.
    int pos = 0;
    for (int i = 0; i < num_avg; i++)
    {
        avg[i] = 0;
    }
    
    // loop through, filling the avg array until returning
    while (1)
    {
        // find the average of every AVG_WINDOW points
        for (int i = 0; i < AVG_WINDOW; i++)
        {
            if (fread(&buffer, sizeof(int16_t), 1, info->fp) != 1)
            {
                return 0;
            }
            avg[pos] += (buffer >= 0) ? (double) buffer : (-1 * (double)buffer);
            if (info->num_channels == 2)
            {
                if (fread(&buffer, sizeof(int16_t), 1, info->fp) != 1)
                {
                    return 0;
                }
            }
        }
        avg[pos] /= AVG_WINDOW;
        pos++;
    }
}


int makeWindow(wavFileInfo* info, double* data_left, double* data_right, int note_length)
{
    // create buffer
    int16_t buffer;
    
    // read the data from .wav file into data arrays
    for (int i = 0; i < note_length; i++)
    {
        if (fread(&buffer, sizeof(int16_t), 1, info->fp) != 1)
        {
            return 1;
        }
        data_left[i] = (double) buffer;
        if (info->num_channels == 2)
        {
            if (fread(&buffer, sizeof(int16_t), 1, info->fp) != 1)
            {
                return 1;
            }
            data_right[i] = (double) buffer;
        }
    }
    return 0;
}

int analyzeData(double data[], FILE* out, wavFileInfo* info, int current_size)
{
    // declarations and initializations
    float frequency = 0.0;
    int max_key_number = 0;
    int key_number = 0;

    // run gsl fft
    gsl_fft_real_wavetable* wavetable = gsl_fft_real_wavetable_alloc(current_size);
    if (wavetable == NULL)
    {
        return -1;
    }
    gsl_fft_real_workspace* workspace = gsl_fft_real_workspace_alloc(current_size);
    if (wavetable == NULL)
    {
        gsl_fft_real_wavetable_free(wavetable);
        return -1;
    }

    gsl_fft_real_transform(data, 1, current_size, wavetable, workspace);

    gsl_fft_real_wavetable_free(wavetable);
    gsl_fft_real_workspace_free(workspace);

    // find the largest frequency component for the left channel
    float max = 0;
    int idx = 0;
    for (int i = 0; i < current_size; i++)
    {
        if (data[i] > max)
        {
            idx = i;
            max = data[i];
        }
    }
    
    // calculate frequency based on fft output
    float base_freq = info->sample_rate / (float)current_size;
    frequency = idx / 2.0 * base_freq;
    max_key_number = round(12 * log2f(frequency / 440) + 49);
    
    // ensure validity of frequency (ignore "blank" segments of noise)
    if (max_key_number < 0 || max_key_number > 88)
    {
        max_key_number = 0;
    }



    // create a histogram for data analysis
    size_t n = 88;
    double range[89] = {0};
    
    // set bin ranges
    for (int i = 0; i < 88; i++)
    {
        range[i] = (pow(2.0, ((i + .5) - 49) / 12.0) * 440.0);
    }
    range[88] = 10000;
    
    gsl_histogram* h = gsl_histogram_alloc(n);
    if (h == NULL)
    {
        return -1;
    }
    
    gsl_histogram_set_ranges(h, range, 89);


    // find the largest NUMMAX frequency components
    for (int j = 0; j < NUMMAX; j++)
    {
        max = 0;
        idx = 0;
        for (int i = 0; i < current_size; i++)
        {
            if (data[i] > max)
            {
                idx = i;
                max = data[i];
            }
        }
        
        // increment correct bin in histogram
        gsl_histogram_increment(h, (idx / 2.0 * base_freq));
        
        // print out top thirty frequencies and amplitudes for analysis
        fprintf(out, "%.0f:%.0f\n", (idx / 2.0 * base_freq), max);
        
        // set highest to zero, then repeat to find next highest
        data[idx] = 0;
    }


    // search for clumps
    key_number = max_key_number + findClumps(h, max_key_number);


    // plot histogram for analysis
    fprintf(out, "\n-----------------------\n");
    char* names[12] = {"    A",
            "A#/Bb",
            "    B",
            "    C",
            "C#/Db",
            "    D",
            "D#/Eb",
            "    E",
            "    F",
            "F#/Gb",
            "    G",
            "G#/Ab"};
    
    for (int i = 0; i < 88; i++)
    {
        fprintf(out, "%s(%d):",  names[i % 12], (i + 1));
        size_t n = (size_t) i;
        double x = gsl_histogram_get(h, n);
        for (int j = 0; j < x; j++)
        {
            fprintf(out, "#");
        }
        fprintf(out, "\n");
    }
    fprintf(out, "\n***********************\n\n");
    
    // free the histogram
    gsl_histogram_free(h);

    // clear data array in preparation for next note
    for (int i = 0; i < current_size; i++)
    {
        data[i] = 0;
    }

    return key_number;
}

int openWavFile(wavFileInfo* info)
{
    // declare a chunk of data (little endian)
    unsigned char chunk[5];
    chunk[4] = '\0';

    // RIFF
    fread(chunk, 4, 1, info->fp);
    if (strcmp((const char*)chunk, "RIFF") != 0)
    {
        printf("ERROR: No RIFF chunk\n");
        return 1;
    }

    // calculate chunk size
    fread(chunk, 4, 1, info->fp);
    info->chunk_size = (chunk[3] << 24) | (chunk[2] << 16) | (chunk[1] << 8) | chunk[0];

    // WAVE
    fread(chunk, 4, 1, info->fp);
    if (strcmp((const char*)chunk, "WAVE") != 0)
    {
        printf("ERROR: No WAVE chunk\n");
        return 1;
    }

    // "fmt "
    fread(chunk, 4, 1, info->fp);
    if (strcmp((const char*)chunk, "fmt ") != 0)
    {
        printf("ERROR: No \"fmt \" chunk\n");
        return 1;
    }

    // calculate chunk size
    fread(chunk, 4, 1, info->fp);
    info->subchunk1_size = (chunk[3] << 24) | (chunk[2] << 16) | (chunk[1] << 8) | chunk[0];

    // audio format and num_channels
    fread(chunk, 4, 1, info->fp);
    info->audio_format = (int) chunk[0];
    info->num_channels = (int) chunk[2];

    // sampling rate
    fread(chunk, 4, 1, info->fp);
    info->sample_rate = (chunk[3] << 24) | (chunk[2] << 16) | (chunk[1] << 8) | chunk[0];

    // byte rate
    fread(chunk, 4, 1, info->fp);
    info->byte_rate = (chunk[3] << 24) | (chunk[2] << 16) | (chunk[1] << 8) | chunk[0];

    // block align and bits per sample
    fread(chunk, 4, 1, info->fp);
    info->block_align = (chunk[1] << 8) | chunk[0];
    info->bits_per_sample = (chunk[3] << 8) | chunk[2];

    // "data"
    fread(chunk, 4, 1, info->fp);
    if (strcmp((const char*)chunk, "data") != 0)
    {
        printf("ERROR: NO        \"data\"\n");
        return 1;
    }

    // block align and bits per sample
    fread(chunk, 4, 1, info->fp);
    info->subchunk2_size = (chunk[3] << 24) | (chunk[2] << 16) | (chunk[1] << 8) | chunk[0];

    // verify appropriate header properties
    if (info->block_align * info->sample_rate != info->byte_rate ||
        (info->bits_per_sample / 8) * info->num_channels != info->block_align || info->chunk_size - 36 != info->subchunk2_size)
    {
        printf("Error in file encoding: mismatch in file header data.");
        return 1;
    }

    return 0;
}

int findClumps(gsl_histogram* h, int max_key)
{
    int check = 0;
    int size = 0;
    int oct = -max_key / 12;
    // check each octave of the maximum amplitude note to find actual notes
    while (check < 88)
    {
        size = 0;
        check = max_key + 12 * oct;
        size_t n = 0;
        // check the two bins around it
        for (int i = check - 1; i <= check + 1; i++)
        {
            if (i < 0)
            {
                size = 0;
                n = 0;
            }
            else if (i > 87)
            {
                break;
            }
            else
            {
                n = (size_t) i;
            }

            size += gsl_histogram_get(h, n);
        }
        // if there is anything in the bins, search for overtones
        if (size >= 1)
        {
            n = (size_t) max_key + oct * 12;
            return oct * 12;
        }
        oct ++;
        check = max_key + 12 * oct;
    }
    return 0;
}

double* diff(double data[], int n)
{
    // take the first discrete derivative of the data vector
    double* data_diff = malloc(sizeof(double) * (n - 1));
    if (data_diff == NULL)
    {
        return NULL;
    }
    
    for (int i = 0; i < n - 1; i++)
    {
        data_diff[i] = data[i + 1] - data[i];
    }
    return data_diff;
}

double max(double data[], int n)
{
    // iteratively find the max of the array
    double max = 0;
    for (int i = 0; i < n; i++)
        if (data[i] > max)
        {
            max = data[i];
        }
    return max;
}

// from "Bit Twiddling Hacks" , http://graphics.stanford.edu/~seander/bithacks.html
int powerOfTwo(int v)
{
    // rounds up to the nearest power of two
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}
