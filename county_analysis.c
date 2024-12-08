// include standard libraries inputs/outputs, types, memory
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

// maximum counties and line length
#define MAX_COUNTIES 5000
#define MAX_LINE_LENGTH 2048

// struct to hold all demographic data
typedef struct {
    char county[100];
    char state[3];
    float education_high_school_or_higher;
    float education_bachelors_or_higher;
    float ethnicities_white_alone;
    float ethnicities_black_alone;
    float ethnicities_asian_alone;
    float ethnicities_american_indian;
    float ethnicities_native_hawaiian;
    float ethnicities_hispanic_or_latino;
    float ethnicities_two_or_more_races;
    float ethnicities_white_alone_not_hispanic;
    int income_median_household;
    int income_per_capita;
    float income_persons_below_poverty_level;
    int population_2014;
} CountyData;

// struct to map the CSV files to fields
typedef struct {
    int field_index;
    size_t offset;
    char type;
    size_t max_size;
} FieldMapping;


// shows all method functionalities
int parse_csv(const char *filename, CountyData *data, int *count);
void process_operations(CountyData *data, int count, const char *ops_file);
void trim_quotes(char *str);
void calculate_population_by_field(CountyData *data, int count, const char *field);
void calculate_percentage(CountyData *data, int count, const char *field);
char *normalize_field(const char *field);
int filter_by_state(CountyData *data, int count, const char *state);
void display_records(CountyData *data, int count);


// map to CSV file for each subset
static const FieldMapping field_mappings[] = {
    {0, offsetof(CountyData, county), 's', sizeof(((CountyData *)0)->county)},
    {1, offsetof(CountyData, state), 's', sizeof(((CountyData *)0)->state)},
    {5, offsetof(CountyData, education_bachelors_or_higher), 'f', 0},
    {6, offsetof(CountyData, education_high_school_or_higher), 'f', 0},
    {17, offsetof(CountyData, ethnicities_white_alone), 'f', 0},
    {13, offsetof(CountyData, ethnicities_black_alone), 'f', 0},
    {12, offsetof(CountyData, ethnicities_asian_alone), 'f', 0},
    {11, offsetof(CountyData, ethnicities_american_indian), 'f', 0},
    {15, offsetof(CountyData, ethnicities_native_hawaiian), 'f', 0},
    {14, offsetof(CountyData, ethnicities_hispanic_or_latino), 'f', 0},
    {16, offsetof(CountyData, ethnicities_two_or_more_races), 'f', 0},
    {18, offsetof(CountyData, ethnicities_white_alone_not_hispanic), 'f', 0},
    {25, offsetof(CountyData, income_median_household), 'i', 0},
    {26, offsetof(CountyData, income_per_capita), 'i', 0},
    {27, offsetof(CountyData, income_persons_below_poverty_level), 'f', 0},
    {38, offsetof(CountyData, population_2014), 'i', 0}
};

// trims the quotes for strings, used for parsing
void trim_quotes(char *str) {
    // iteratre through the string and remove queotes
    if (str[0] == '"' && str[strlen(str) - 1] == '"') {
        str[strlen(str) - 1] = '\0';
        memmove(str, str + 1, strlen(str));
    }
}

// makes all the header names similar so manipulation is easy
char *normalize_field(const char *field) {
    if (strcmp(field, "Education.Bachelor's Degree or Higher") == 0)
        return "education_bachelors_or_higher";
    if (strcmp(field, "Education.High School or Higher") == 0)
        return "education_high_school_or_higher";
    if (strcmp(field, "Ethnicities.White Alone") == 0)
        return "ethnicities_white_alone";
    if (strcmp(field, "Ethnicities.Black Alone") == 0)
        return "ethnicities_black_alone";
    if (strcmp(field, "Ethnicities.Asian Alone") == 0)
        return "ethnicities_asian_alone";
    if (strcmp(field, "Ethnicities.American Indian and Alaska Native Alone") == 0)
        return "ethnicities_american_indian";
    if (strcmp(field, "Ethnicities.Native Hawaiian and Other Pacific Islander Alone") == 0)
        return "ethnicities_native_hawaiian";
    if (strcmp(field, "Ethnicities.Hispanic or Latino") == 0)
        return "ethnicities_hispanic_or_latino";
    if (strcmp(field, "Ethnicities.Two or More Races") == 0)
        return "ethnicities_two_or_more_races";
    if (strcmp(field, "Ethnicities.White Alone, not Hispanic or Latino") == 0)
        return "ethnicities_white_alone_not_hispanic";
    if (strcmp(field, "Income.Persons Below Poverty Level") == 0)
        return "income_persons_below_poverty_level";
    return NULL;
}

// parses the csv file into the CountyData struct
// takes 3 args
void process_token(CountyData *entry, const char *token, int field_index) {
    // iteratre through and map the data based on the type
    for (size_t i = 0; i < sizeof(field_mappings) / sizeof(field_mappings[0]); i++) {
        if (field_mappings[i].field_index == field_index) {
            char type = field_mappings[i].type;
            void *field_ptr = (char *)entry + field_mappings[i].offset;
            // for integer
            if (type == 'i') { 
                *(int *)field_ptr = atoi(token);
            // for float
            } else if (type == 'f') { 
                *(float *)field_ptr = atof(token);
            // for string
            } else if (type == 's') {
                // to null terminate
                strncpy((char *)field_ptr, token, field_mappings[i].max_size - 1);
                ((char *)field_ptr)[field_mappings[i].max_size - 1] = '\0'; 
            }
            break;
        }
    }
}

// reads data from the csv file nad puts into arr
int parse_csv(const char *filename, CountyData *data, int *count) {
    // open file in read only
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return -1;
    }
    char line[MAX_LINE_LENGTH];
    // keep track of current number
    int line_number = 0;
    // keeps track of count
    int valid_count = 0;
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        // to skip header
        if (line_number == 1) {
            continue;
        }
        CountyData entry = {0};
        char *token = strtok(line, ",");
        int field_index = 0;
        // processes each in line
        while (token) {
            trim_quotes(token);
            process_token(&entry, token, field_index);
            token = strtok(NULL, ",");
            field_index++;
        }
        // make sure country and state aren't empty
        if (strlen(entry.county) == 0 || strlen(entry.state) == 0) {
            fprintf(stderr, "malfomrmed line %d: fields\n", line_number);
            continue;
        }
        // check max limit
        if (valid_count >= MAX_COUNTIES) {
            fprintf(stderr, "error too many entries (max %d reached)\n", MAX_COUNTIES);
            break;
        }
        // store the entry if valid
        data[valid_count++] = entry;
    }

    fclose(file);
    *count = valid_count;
    return 0;
}

// function filters by state as specified in spec
int filter_by_state(CountyData *data, int count, const char *state) {
    int new_count = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(data[i].state, state) == 0) {
            data[new_count++] = data[i];
        }
    }
    printf("Filter: state == %s (%d entries)\n", state, new_count);
    return new_count;
}

// functin filters by the field specified for the .ops files
int filter_by_field(CountyData *data, int count, const char *field, const char *op, float value) {
    // keeps track of new records
    int new_count = 0;
    // iterate through to check for fields
    for (int i = 0; i < count; i++) {
        float field_value = 0;
        if (strcmp(field, "education_high_school_or_higher") == 0)
            field_value = data[i].education_high_school_or_higher;
        else if (strcmp(field, "education_bachelors_or_higher") == 0)
            field_value = data[i].education_bachelors_or_higher;
        else if (strcmp(field, "ethnicities_hispanic_or_latino") == 0)
            field_value = data[i].ethnicities_hispanic_or_latino;
        else {
            fprintf(stderr, "unsupported field: %s\n", field);
            return count;
        }

        int match = 0;
        if (strcmp(op, "le") == 0)
            match = field_value <= value;
        else if (strcmp(op, "ge") == 0)
            match = field_value >= value;

        if (match)
            data[new_count++] = data[i];
    }
    // print based on the tupe
    printf("Filter: %s %s %.6f (%d entries)\n", field, op, value, new_count);
    return new_count;
}

// process all operations for the .ops files
void process_operations(CountyData *data, int count, const char *ops_file) {
    // open in read only mode
    FILE *file = fopen(ops_file, "r");
    if (!file) {
        perror("error opening file");
        return;
    }
    char line[MAX_LINE_LENGTH];
    // while loop to continue till flag
    while (fgets(line, sizeof(line), file)) {
        // strips newline
        line[strcspn(line, "\r\n")] = '\0'; 
        if (strncmp(line, "filter:", 7) == 0) {
            // handle the filter sets based on initial struct
            char field[100], op[3];
            float value;
            // to parse operations
            sscanf(line + 7, "%[^:]:%2[^:]:%f", field, op, &value);
            char *normalized_field = normalize_field(field);
            if (normalized_field) {
                count = filter_by_field(data, count, normalized_field, op, value);
            } else {
                fprintf(stderr, "field not valid: %s\n", field);
            }
        } else if (strcmp(line, "display") == 0) {
            // if display is called in .ops
            display_records(data, count);
        } else if (strncmp(line, "population:", 11) == 0) {
            // calls normalize function
            char *field = normalize_field(line + 11);
            if (field) {
                calculate_population_by_field(data, count, field);
            } else {
                fprintf(stderr, "field not valid: %s\n", line + 11);
            }
        } else if (strncmp(line, "percent:", 8) == 0) {
            char *field = normalize_field(line + 8);
            if (field) {
                calculate_percentage(data, count, field);
            } else {
                fprintf(stderr, "field not valid: %s\n", line + 8);
            }
        } else if (strcmp(line, "population-total") == 0) {
            int total_population = 0;
            for (int i = 0; i < count; i++) {
                total_population += data[i].population_2014;
            }
            printf("2014 population: %d\n", total_population);
        } else if (strncmp(line, "filter-state:", 13) == 0) {
            count = filter_by_state(data, count, line + 13);
        } else {
            fprintf(stderr, "operation not valid: %s\n", line);
        }
    }
    fclose(file);
}

// calculates the population based on field for .ops
void calculate_population_by_field(CountyData *data, int count, const char *field) {
    double total_population = 0;
    // iterate through for the rows
    for (int i = 0; i < count; i++) {
        float percentage = 0;
        if (strcmp(field, "education_bachelors_or_higher") == 0) {
            percentage = data[i].education_bachelors_or_higher;
        } else if (strcmp(field, "education_high_school_or_higher") == 0) {
            percentage = data[i].education_high_school_or_higher;
        } else if (strcmp(field, "ethnicities_white_alone") == 0) {
            percentage = data[i].ethnicities_white_alone;
        } else if (strcmp(field, "ethnicities_black_alone") == 0) {
            percentage = data[i].ethnicities_black_alone;
        } else if (strcmp(field, "ethnicities_asian_alone") == 0) {
            percentage = data[i].ethnicities_asian_alone;
        } else if (strcmp(field, "ethnicities_american_indian") == 0) {
            percentage = data[i].ethnicities_american_indian;
        } else if (strcmp(field, "ethnicities_native_hawaiian") == 0) {
            percentage = data[i].ethnicities_native_hawaiian;
        } else if (strcmp(field, "ethnicities_hispanic_or_latino") == 0) {
            percentage = data[i].ethnicities_hispanic_or_latino;
        } else if (strcmp(field, "ethnicities_two_or_more_races") == 0) {
            percentage = data[i].ethnicities_two_or_more_races;
        } else if (strcmp(field, "ethnicities_white_alone_not_hispanic") == 0) {
            percentage = data[i].ethnicities_white_alone_not_hispanic;
        } else if (strcmp(field, "income_persons_below_poverty_level") == 0) {
            percentage = data[i].income_persons_below_poverty_level;
        } else {
            fprintf(stderr, "field not valid: %s\n", field);
            return;
        }
        // calculate field population
        total_population += data[i].population_2014 * (percentage / 100.0);
    }
    // print population
    printf("2014 %s population: %.6f\n", field, total_population);
}

// displays the records as specified in the specifications
void display_records(CountyData *data, int count) {
    // iterates through to print i number of data subsets
    for (int i = 0; i < count; i++) {
        printf("%s, %s\n", data[i].county, data[i].state);
        printf("    Population: %d\n", data[i].population_2014);
        printf("    Education\n");
        printf("        >= High School: %.6f%%\n", data[i].education_high_school_or_higher);
        printf("        >= Bachelor's: %.6f%%\n", data[i].education_bachelors_or_higher);
        printf("    Ethnicity Percentages\n");
        printf("        White Alone: %.6f%%\n", data[i].ethnicities_white_alone);
        printf("        Black Alone: %.6f%%\n", data[i].ethnicities_black_alone);
        printf("        Asian Alone: %.6f%%\n", data[i].ethnicities_asian_alone);
        printf("        Hispanic or Latino: %.6f%%\n", data[i].ethnicities_hispanic_or_latino);
        printf("    Income\n");
        printf("        Median Household: %d\n", data[i].income_median_household);
        printf("        Per Capita: %d\n", data[i].income_per_capita);
        printf("        Below Poverty Level: %.6f%%\n", data[i].income_persons_below_poverty_level);
        printf("\n");
    }
}


// calculate percentage for .ops files
void calculate_percentage(CountyData *data, int count, const char *field) {
    // holders to calculate percentage
    double sub_population = 0;
    int total_population = 0;
    // iterate through rows to get values
    for (int i = 0; i < count; i++) {
        total_population += data[i].population_2014;
        float percentage = 0;
        if (strcmp(field, "education_bachelors_or_higher") == 0) {
            percentage = data[i].education_bachelors_or_higher;
        } else if (strcmp(field, "education_high_school_or_higher") == 0) {
            percentage = data[i].education_high_school_or_higher;
        } else if (strcmp(field, "ethnicities_white_alone") == 0) {
            percentage = data[i].ethnicities_white_alone;
        } else if (strcmp(field, "ethnicities_black_alone") == 0) {
            percentage = data[i].ethnicities_black_alone;
        } else if (strcmp(field, "ethnicities_asian_alone") == 0) {
            percentage = data[i].ethnicities_asian_alone;
        } else if (strcmp(field, "ethnicities_american_indian") == 0) {
            percentage = data[i].ethnicities_american_indian;
        } else if (strcmp(field, "ethnicities_native_hawaiian") == 0) {
            percentage = data[i].ethnicities_native_hawaiian;
        } else if (strcmp(field, "ethnicities_hispanic_or_latino") == 0) {
            percentage = data[i].ethnicities_hispanic_or_latino;
        } else if (strcmp(field, "ethnicities_two_or_more_races") == 0) {
            percentage = data[i].ethnicities_two_or_more_races;
        } else if (strcmp(field, "ethnicities_white_alone_not_hispanic") == 0) {
            percentage = data[i].ethnicities_white_alone_not_hispanic;
        } else if (strcmp(field, "income_persons_below_poverty_level") == 0) {
            percentage = data[i].income_persons_below_poverty_level;
        } else {
            fprintf(stderr, "field not valid: %s\n", field);
            return;
        }
        // calculate poopulation
        sub_population += data[i].population_2014 * (percentage / 100.0);
    }
    if (total_population > 0) {
        // divide for percentage and divide
        double field_percentage = (sub_population / total_population) * 100.0;
        printf("2014 %s percentage: %.6f\n", field, field_percentage);
    } else {
        fprintf(stderr, "Error: Total population is zero\n");
    }
}

// main function
int main(int argc, char *argv[]) {
    if (argc != 3) {
        // print usage args in case of error as specified in spec
        fprintf(stderr, "Usage: %s <csv_file> <ops_file>\n", argv[0]);
        return 1;
    }
    // array holds all the county data
    CountyData data[MAX_COUNTIES];
    int count = 0;
    // prase file unless error
    if (parse_csv(argv[1], data, &count) != 0) {
        fprintf(stderr, "CSV file is not parsable\n");
        return 1;
    }
    // print # of records
    printf("%d records loaded\n", count);
    process_operations(data, count, argv[2]);
    return 0;
}

