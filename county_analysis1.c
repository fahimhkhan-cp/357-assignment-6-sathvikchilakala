#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_COUNTIES 5000
#define MAX_LINE_LENGTH 2048

// Define a structure to hold the county demographics data
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

// Function prototypes
int parse_csv(const char *filename, CountyData *data, int *count);
void process_operations(CountyData *data, int count, const char *ops_file);
void trim_quotes(char *str);
void calculate_population_by_field(CountyData *data, int count, const char *field);
void calculate_percentage(CountyData *data, int count, const char *field);
char *normalize_field(const char *field);
int filter_by_state(CountyData *data, int count, const char *state);
void display_records(CountyData *data, int count);



// Function to trim quotes from strings
void trim_quotes(char *str) {
    if (str[0] == '"' && str[strlen(str) - 1] == '"') {
        str[strlen(str) - 1] = '\0';
        memmove(str, str + 1, strlen(str));
    }
}

// Normalize field names for operations
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

// Parse the CSV file into CountyData structs
int parse_csv(const char *filename, CountyData *data, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return -1;
    }
    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    int valid_count = 0;
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        if (line_number == 1) // Skip header
            continue;

        char *token;
        int field_index = 0;
        CountyData entry = {0};
        token = strtok(line, ",");
        while (token) {
            trim_quotes(token);
            switch (field_index) {
                case 0: strncpy(entry.county, token, sizeof(entry.county) - 1); break;
                case 1: strncpy(entry.state, token, sizeof(entry.state) - 1); break;
                case 5: entry.education_bachelors_or_higher = atof(token); break;
                case 6: entry.education_high_school_or_higher = atof(token); break;
                case 17: entry.ethnicities_white_alone = atof(token); break;
                case 13: entry.ethnicities_black_alone = atof(token); break;
                case 12: entry.ethnicities_asian_alone = atof(token); break;
                case 11: entry.ethnicities_american_indian = atof(token); break;
                case 15: entry.ethnicities_native_hawaiian = atof(token); break;
                case 14: entry.ethnicities_hispanic_or_latino = atof(token); break;
                case 16: entry.ethnicities_two_or_more_races = atof(token); break;
                case 18: entry.ethnicities_white_alone_not_hispanic = atof(token); break;
                case 25: entry.income_median_household = atoi(token); break;
                case 26: entry.income_per_capita = atoi(token); break;
                case 27: entry.income_persons_below_poverty_level = atof(token); break;
                case 38: entry.population_2014 = atoi(token); break;
            }
            token = strtok(NULL, ",");
            field_index++;
        }

        if (strlen(entry.county) == 0 || strlen(entry.state) == 0) {
            fprintf(stderr, "Malformed line %d: insufficient fields\n", line_number);
            continue;
        }

        if (valid_count >= MAX_COUNTIES) {
            fprintf(stderr, "Error: Too many entries to load (max %d reached)\n", MAX_COUNTIES);
            break;
        }
        data[valid_count++] = entry;
    }
    fclose(file);
    *count = valid_count;
    return 0;
}

// Filter by state
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


// Process operations from the operations file
void process_operations(CountyData *data, int count, const char *ops_file) {
    FILE *file = fopen(ops_file, "r");
    if (!file) {
        perror("Error opening operations file");
        return;
    }
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0'; // Strip newline
        if (strncmp(line, "population:", 11) == 0) {
            char *field = normalize_field(line + 11);
            if (field) {
                calculate_population_by_field(data, count, field);
            } else {
                fprintf(stderr, "Unsupported field: %s\n", line + 11);
            }
        } else if (strncmp(line, "percent:", 8) == 0) {
            char *field = normalize_field(line + 8);
            if (field) {
                calculate_percentage(data, count, field);
            } else {
                fprintf(stderr, "Unsupported field: %s\n", line + 8);
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
            fprintf(stderr, "Unsupported operation: %s\n", line);
        }
    }
    fclose(file);
}

// Calculate population by field
void calculate_population_by_field(CountyData *data, int count, const char *field) {
    double total_population = 0;
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
            fprintf(stderr, "Unsupported field: %s\n", field);
            return;
        }
        total_population += data[i].population_2014 * (percentage / 100.0);
    }
    printf("2014 %s population: %.6f\n", field, total_population);
}

// Calculate percentage by field
void calculate_percentage(CountyData *data, int count, const char *field) {
    double sub_population = 0;
    int total_population = 0;

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
            fprintf(stderr, "Unsupported field: %s\n", field);
            return;
        }
        sub_population += data[i].population_2014 * (percentage / 100.0);
    }

    if (total_population > 0) {
        double field_percentage = (sub_population / total_population) * 100.0;
        printf("2014 %s percentage: %.6f\n", field, field_percentage);
    } else {
        fprintf(stderr, "Error: Total population is zero\n");
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <csv_file> <ops_file>\n", argv[0]);
        return 1;
    }

    CountyData data[MAX_COUNTIES];
    int count = 0;

    if (parse_csv(argv[1], data, &count) != 0) {
        fprintf(stderr, "Failed to parse CSV file\n");
        return 1;
    }

    printf("%d records loaded\n", count);
    process_operations(data, count, argv[2]);
    return 0;
}

