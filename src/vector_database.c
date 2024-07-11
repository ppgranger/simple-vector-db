#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "vector_database.h"

VectorDatabase* vector_db_init(size_t initial_capacity) {
    VectorDatabase* db = (VectorDatabase*)malloc(sizeof(VectorDatabase));
    if (!db) {
        fprintf(stderr, "Failed to allocate memory for database\n");
        return NULL;
    }
    db->size = 0;
    db->capacity = initial_capacity > 0 ? initial_capacity : 10;
    db->vectors = (Vector*)malloc(db->capacity * sizeof(Vector));
    if (!db->vectors) {
        fprintf(stderr, "Failed to allocate memory for vectors\n");
        free(db);
        return NULL;
    }
    return db;
}

void vector_db_free(VectorDatabase* db) {
    if (db) {
        for (size_t i = 0; i < db->size; ++i) {
            free(db->vectors[i].data);
        }
        free(db->vectors);
        free(db);
    }
}

size_t vector_db_insert(VectorDatabase* db, Vector vec) {
    if (db->size >= db->capacity) {
        db->capacity *= 2;
        Vector* new_vectors = (Vector*)realloc(db->vectors, db->capacity * sizeof(Vector));
        if (!new_vectors) {
            fprintf(stderr, "Failed to allocate more memory for vectors\n");
            return (size_t)-1; // Return -1 on failure
        }
        db->vectors = new_vectors;
    }
    vec.median_point = calculate_median(vec.data, vec.dimension); // Calculate and store the median point
    db->vectors[db->size] = vec;
    printf("Inserted vector at index %zu with dimension %zu and median_point %f\n", db->size - 1, vec.dimension, vec.median_point);
    return db->size++;
}

Vector* vector_db_read(VectorDatabase* db, size_t index) {
    if (index < db->size) {
        return &db->vectors[index];
    }
    return NULL;
}

void vector_db_update(VectorDatabase* db, size_t index, Vector vec) {
    if (index < db->size) {
        free(db->vectors[index].data);
        vec.median_point = calculate_median(vec.data, vec.dimension); // Calculate and store the median point
        db->vectors[index] = vec;
    }
}

void vector_db_delete(VectorDatabase* db, size_t index) {
    if (index < db->size) {
        free(db->vectors[index].data);
        for (size_t i = index; i < db->size - 1; ++i) {
            db->vectors[i] = db->vectors[i + 1];
        }
        db->size--;
    }
}

void vector_db_save(VectorDatabase* db, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    printf("Saving database of size %zu\n", db->size);
    fwrite(&db->size, sizeof(size_t), 1, file);
    for (size_t i = 0; i < db->size; ++i) {
        if (db->vectors[i].dimension == 0 || db->vectors[i].data == NULL) {
            fprintf(stderr, "Invalid vector at index %zu, skipping\n", i);
            continue;
        }
        printf("Saving vector at index %zu with dimension %zu\n", i, db->vectors[i].dimension);
        fwrite(&db->vectors[i].dimension, sizeof(size_t), 1, file);
        fwrite(db->vectors[i].data, sizeof(double), db->vectors[i].dimension, file);
        fwrite(&db->vectors[i].median_point, sizeof(double), 1, file); // Save the median point
    }

    fclose(file);
    printf("Database saved to %s\n", filename);
}

VectorDatabase* vector_db_load(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file for reading");
        return NULL;
    }

    VectorDatabase* db = (VectorDatabase*)malloc(sizeof(VectorDatabase));
    if (!db) {
        fprintf(stderr, "Failed to allocate memory for database\n");
        fclose(file);
        return NULL;
    }

    fread(&db->size, sizeof(size_t), 1, file);
    db->capacity = db->size > 0 ? db->size : 10;
    db->vectors = (Vector*)malloc(db->capacity * sizeof(Vector));
    if (!db->vectors) {
        fprintf(stderr, "Failed to allocate memory for vectors\n");
        free(db);
        fclose(file);
        return NULL;
    }

    for (size_t i = 0; i < db->size; ++i) {
        fread(&db->vectors[i].dimension, sizeof(size_t), 1, file);
        db->vectors[i].data = (double*)malloc(db->vectors[i].dimension * sizeof(double));
        if (!db->vectors[i].data) {
            fprintf(stderr, "Failed to allocate memory for vector data\n");
            vector_db_free(db);
            fclose(file);
            return NULL;
        }
        fread(db->vectors[i].data, sizeof(double), db->vectors[i].dimension, file);
        fread(&db->vectors[i].median_point, sizeof(double), 1, file); // Load the median point
    }

    fclose(file);
    printf("Database loaded from %s\n", filename);
    return db;
}

float cosine_similarity(Vector vec1, Vector vec2) {
    if (vec1.dimension != vec2.dimension) {
        fprintf(stderr, "Vectors have different dimensions\n");
        return -1.0;
    }
    float dot_product = 0.0, norm_a = 0.0, norm_b = 0.0;
    for (size_t i = 0; i < vec1.dimension; i++) {
        dot_product += vec1.data[i] * vec2.data[i];
        norm_a += vec1.data[i] * vec1.data[i];
        norm_b += vec2.data[i] * vec2.data[i];
    }
    return dot_product / (sqrt(norm_a) * sqrt(norm_b));
}

float euclidean_distance(Vector vec1, Vector vec2) {
    if (vec1.dimension != vec2.dimension) {
        fprintf(stderr, "Vectors have different dimensions\n");
        return -1.0;
    }
    float sum = 0.0;
    for (size_t i = 0; i < vec1.dimension; i++) {
        float diff = vec1.data[i] - vec2.data[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

float dot_product(Vector vec1, Vector vec2) {
    if (vec1.dimension != vec2.dimension) {
        fprintf(stderr, "Vectors have different dimensions\n");
        return -1.0;
    }
    float result = 0.0;
    for (size_t i = 0; i < vec1.dimension; i++) {
        result += vec1.data[i] * vec2.data[i];
    }
    return result;
}

double calculate_median(double* data, size_t dimension) {
    // Simple test - need to implement kd-tree later
    if (dimension % 2 == 0) {
        return (data[dimension / 2 - 1] + data[dimension / 2]) / 2.0;
    } else {
        return data[dimension / 2];
    }
}