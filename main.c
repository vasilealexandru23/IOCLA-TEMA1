#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operations.h"
#define MAX_LEN_CMD 100

sensor *read_data(const char *filename, int *number_sensors)
{
	// Deschidem fisierul binar din
	// care citim datele senzorilor.
	FILE *fin = fopen(filename, "rb");
	DIE(!fin, "Failed to open file!");
	fread(number_sensors, sizeof(int), 1, fin);

	// Alocam cei doi vectori pentru cele doua tipuri de senzori.
	int index_ts = 0, index_pmu = 0;
	sensor *tire_sensors = malloc((*number_sensors) * sizeof(sensor));
	DIE(!tire_sensors, "Failed to malloc tire_sensors!");
	sensor *pmu_sensors = malloc((*number_sensors) * sizeof(sensor));
	DIE(!pmu_sensors, "Failed to malloc pmu_sensors!");

	// Citim datele fiecarui tip de senzor si
	// adaugam fiecare element in array-ul corespunzator.
	for (int i = 0; i < (*number_sensors); ++i) {
		int type_sensor;
		fread(&type_sensor, sizeof(int), 1, fin);
		if (type_sensor == 0) {
			tire_sensors[index_ts].sensor_type = TIRE;
			tire_sensors[index_ts].sensor_data = malloc(sizeof(tire_sensor));

			tire_sensor curr_sensor;
			fread(&curr_sensor, sizeof(curr_sensor), 1, fin);
			memcpy(tire_sensors[index_ts].sensor_data, &curr_sensor,
			       sizeof(curr_sensor));

			fread(&tire_sensors[index_ts].nr_operations, sizeof(int), 1, fin);
			tire_sensors[index_ts].operations_idxs = malloc(
			    tire_sensors[index_ts].nr_operations * sizeof(int));
			fread(tire_sensors[index_ts].operations_idxs,
			      sizeof(int), tire_sensors[index_ts].nr_operations, fin);

			index_ts++;
		} else {
			pmu_sensors[index_pmu].sensor_type = PMU;
			pmu_sensors[index_pmu].sensor_data =
			    malloc(sizeof(power_management_unit));

			power_management_unit curr_sensor;
			fread(&curr_sensor, sizeof(curr_sensor), 1, fin);
			memcpy(pmu_sensors[index_pmu].sensor_data, &curr_sensor,
			       sizeof(curr_sensor));

			fread(&pmu_sensors[index_pmu].nr_operations, sizeof(int), 1, fin);
			pmu_sensors[index_pmu].operations_idxs = malloc(
			    pmu_sensors[index_pmu].nr_operations * sizeof(int));
			fread(pmu_sensors[index_pmu].operations_idxs,
			      sizeof(int), pmu_sensors[index_pmu].nr_operations, fin);

			index_pmu++;
		}
	}

	fclose(fin);

	// Dam merge la cele doua array-uri de senzori
	// intr-un array care contine ambele tipturi de senzori.
	sensor *sensors = malloc((*number_sensors) * sizeof(sensor));
	DIE(!sensors, "Failed to malloc sensors!");
	int index_sensors = 0;

	for (int i = 0; i < index_pmu; ++i) {
		sensors[index_sensors++] = pmu_sensors[i];
	}

	for (int i = 0; i < index_ts; ++i) {
		sensors[index_sensors++] = tire_sensors[i];
	}

	free(tire_sensors);
	free(pmu_sensors);
	return sensors;
}

void print(sensor *sensors, int index)
{
	// Verificam tipurile de senzori si afisam
	// pentru fiecare tip informatiile corespunzatoare.
	if (sensors[index].sensor_type == TIRE) {
		tire_sensor curr_sensor = *((tire_sensor *)sensors[index].sensor_data);
		printf("Tire Sensor\n");
		printf("Pressure: %.2f\n", curr_sensor.pressure);
		printf("Temperature: %.2f\n", curr_sensor.temperature);
		printf("Wear Level: %d%%\n", curr_sensor.wear_level);

		// Verificam daca am calculat performance_score.
		if (curr_sensor.performace_score)
			printf("Performance Score: %d\n", curr_sensor.performace_score);
		else
			printf("Performance Score: Not Calculated\n");
	} else {
		power_management_unit curr_sensor =
		    *((power_management_unit *)sensors[index].sensor_data);
		printf("Power Management Unit\n");
		printf("Voltage: %.2f\n", curr_sensor.voltage);
		printf("Current: %.2f\n", curr_sensor.current);
		printf("Power Consumption: %.2f\n", curr_sensor.power_consumption);
		printf("Energy Regen: %d%%\n", curr_sensor.energy_regen);
		printf("Energy Storage: %d%%\n", curr_sensor.energy_storage);
	}
}

void remove_sensor(sensor **sensors, int *number_sensors, int *index)
{
	sensor to_erase = (*sensors)[*index];
	// Mutam la stanga cu o pozitie toate
	// elementele din vectorul de senzori.
	for (int sensor = *index; sensor < *number_sensors - 1; ++sensor)
		(*sensors)[sensor] = (*sensors)[sensor + 1];
	(*sensors)[*number_sensors - 1] = to_erase;
	free(to_erase.sensor_data);
	free(to_erase.operations_idxs);
	sensor *tmp = realloc((*sensors), (*number_sensors) * sizeof(sensor));
	if (tmp)
		*sensors = tmp;
	(*number_sensors)--;
	(*index)--;
}

void clear(sensor **sensors, int *number_sensors)
{
	// Parcurgem vectorul de senzori si ii stergem pe cei invalizi.
	for (int index = 0; index < *number_sensors; ++index) {
		if ((*sensors)[index].sensor_type == TIRE) {
			tire_sensor curr_sensor =
			    *((tire_sensor *)(*sensors)[index].sensor_data);
			if (!(curr_sensor.pressure >= 19.0 && curr_sensor.pressure <= 28.0))
				remove_sensor(sensors, number_sensors, &index);
			else if (!(curr_sensor.temperature >= 0.0 &&
				   curr_sensor.temperature <= 120.0))
				remove_sensor(sensors, number_sensors, &index);
			else if (!(curr_sensor.wear_level >= 0 &&
				   curr_sensor.wear_level <= 100))
				remove_sensor(sensors, number_sensors, &index);
		} else {
			power_management_unit curr_sensor =
					*((power_management_unit *)(*sensors)[index].sensor_data);
			if (!(curr_sensor.voltage >= 10.0 && curr_sensor.voltage <= 20.0))
				remove_sensor(sensors, number_sensors, &index);
			else if (!(curr_sensor.current >= -100.0 &&
				   curr_sensor.current <= 100.0))
				remove_sensor(sensors, number_sensors, &index);
			else if (!(curr_sensor.power_consumption >= 0.0 &&
				   curr_sensor.power_consumption <= 1000.0))
				remove_sensor(sensors, number_sensors, &index);
			else if (!(curr_sensor.energy_regen >= 0 &&
				   curr_sensor.energy_regen <= 100))
				remove_sensor(sensors, number_sensors, &index);
			else if (!(curr_sensor.energy_storage >= 0 &&
				   curr_sensor.energy_storage <= 100))
				remove_sensor(sensors, number_sensors, &index);
		}
	}
}

void free_sensors(sensor *sensors, int number_sensors)
{
	// Eliberam memoria alocata
	// fiecarui element din vectorul de senzori.
	for (int i = 0; i < number_sensors; ++i) {
		free(sensors[i].sensor_data);
		free(sensors[i].operations_idxs);
	}
	free(sensors);
	sensors = NULL;
}

void analyze(sensor *sensors, int index)
{
	// Rulam operatiile corespunzatoare.
	void (*vector_of_operations[8])(void *);
	get_operations((void *)vector_of_operations);
	for (int op = 0; op < sensors[index].nr_operations; ++op)
		vector_of_operations[sensors[index].operations_idxs[op]]
		    (sensors[index].sensor_data);
}

int main(int argc, char const *argv[])
{
	// Citim informatiile despre vectorul de senzori.
	int number_sensors;
	sensor *sensors = read_data(argv[1], &number_sensors);

	// Citim comenzile date de
	// utilizator pana la comanda "EXIT".
	char command[MAX_LEN_CMD];
	while (1) {
		scanf("%s", command);
		if (!strcmp(command, "print")) {
			int index;
			scanf("%d", &index);
			if (index < 0 || index > number_sensors) {
				printf("Index not in range!\n");
			} else {
				print(sensors, index);
			}
		} else if (!strcmp(command, "clear")) {
			clear(&sensors, &number_sensors);
		} else if (!strcmp(command, "analyze")) {
			int index;
			scanf("%d", &index);
			if (index < 0 || index > number_sensors) {
				printf("Index not in range!\n");
			} else {
				analyze(sensors, index);
			}
		} else if (!strcmp(command, "exit")) {
			free_sensors(sensors, number_sensors);
			break;
		}
	}
	return 0;
}

