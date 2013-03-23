#include "musicxml.h"

void parseString(char* string, char find, char replace);

int main(int argc, char* argv[])
{
    // usage
    if (argc != 12)
    {
        printf("USAGE: import [input .wav] [output .xml] [key] [new key] [bpm] [meter] [pickup] [harmonic rhythm] [# parts] [composer] [title]\n");
        return 1;
    }
   
    char* in_file = argv[1];
    char* out_file = argv[2];
    int key = atoi(argv[3]);
    int new_key = atoi(argv[4]);
    int bpm = atoi(argv[5]);
    int beats = atoi(argv[6]);
    int pickup = atoi(argv[7]);
    int harmonic_rhythm = atoi(argv[8]);
    int num_parts = atoi(argv[9]);
    char* composer = argv[10];
    char* title = argv[11];
    
    // change underscore to space for the names and titles
    parseString(composer, '_', ' ');
    parseString(title, '_', ' ');

    // error checking
    int in_filename_length = strlen(in_file);
    int out_filename_length = strlen(out_file);
    if (strcmp(&in_file[in_filename_length - 4], ".wav") != 0)
    {
        printf("Error: input file format must be .wav\n");
        return 1;
    }
    if (strcmp(&out_file[out_filename_length - 4], ".xml") != 0)
    {
        printf("Error: output file extension must be .xml\n");
        return 1;
    }
    if (key < -7 || key > 7 || new_key < -7 || key > 7)
    {
        printf("Error: unsupported key\n");
        return 1;
    }
    if (bpm < 20 || bpm > 200)
    {
        printf("Error: unsupported bpm\n");
        return 1;
    }
    if (beats != 3 && beats != 4)
    {
        printf("Error: unsupported time signature\n");
        return 1;
    }
    if (harmonic_rhythm < 0 || harmonic_rhythm > 3)
    {
        printf("Error: unsupported harmonic_rhythm\n");
        return 1;
    }
    if (num_parts < 1 || num_parts > 4)
    {
        printf("Error: unsupported number of parts\n");
        return 1;
    }

    // import a part from tyler
    Part* melody = read(argv[1], bpm, beats * DIVISIONS / NOTESCALEFACTOR);
    if (melody == NULL)
    {
        printf("Error importing melody\n");
        return 1;
    }

    // see what meter it's in. this is not used now, but can be used in the future.
    int* meter_attributes = determineMeter(melody);
    if (meter_attributes == 0)
    {
        printf("Error determining the meter of the melody\n");
        rmPart(melody);
        return 1;
    }

    // offset the start with a pickup measure
    melody = addPickup(melody, pickup, beats);

    // transpose to c to make things easy
    transpose(melody, key, 0, -1);

    // see how long the melody is
    int total_duration = 0;
    Part* melody_ptr = melody;
    while (melody_ptr != NULL)
    {
        total_duration += melody_ptr->duration;
        melody_ptr = melody_ptr->next;
    }

    // These are the options for writing harmonic rhythms.
    Rhythm* rhythm[4];
    rhythm[0] = getRhythm(total_duration, 4, 0);
    rhythm[1] = getRhythm(total_duration, 3, 0);
    rhythm[2] = getRhythm(total_duration, 2, 0);
    rhythm[3] = copyPartRhythm(melody);

    // determine a harmony
    Harmony* my_harmony = determineHarmony(melody, rhythm[harmonic_rhythm], 0, beats);
    if (my_harmony == NULL)
    {
        printf("Error writing imported harmony\n");
        rmPart(melody);
        for (int i = 0; i < 4; i++)
            rmRhythm(rhythm[i]);
        return 1;
    }

    // figure out the other parts
    Part* parts[num_parts];
    parts[0] = melody;
    for (int i = 1; i < num_parts; i++)
        parts[i] = getCounterpointPart(my_harmony, rhythm[harmonic_rhythm], parts, i, 0, 2);

    // make sure the parts were created correctly
    for (int i = 0; i < num_parts; i++)
    {
        if (parts[i] == NULL)
        {
            printf("Error writing harmony parts\n");
            for (int j = 0; j < i; j++)
                rmPart(parts[i]);
            for (int j = 0; j < 4; j++)
                rmRhythm(rhythm[i]);
            rmHarmony(my_harmony);
            return 1;
        }
    }

    // transpose to desired key
    for (int i = 0; i < num_parts; i++)
        transpose(parts[i], 0, new_key, 1);

    // write the part to file
    writePart(out_file, parts, num_parts, beats, new_key, composer, title);

    // free memory
    rmHarmony(my_harmony);
    for (int i = 0; i < 4; i++)
        rmRhythm(rhythm[i]);
    for (int i = 0; i < num_parts; i++)
        rmPart(parts[i]);

    // open up the result in finale notepad
    char* open = "open -a /Applications/Finale\\ NotePad\\ 2012.app ";
    char full_command[(strlen(open) + strlen(out_file) + 1)];
    sprintf(full_command, "%s%s", open, out_file);
    system(full_command);

    return 0;
}


void parseString(char* string, char find, char replace)
{
    for (int i = 0; string[i] != '\0'; i++)
        if (string[i] == find)
            string[i] = replace;
}
