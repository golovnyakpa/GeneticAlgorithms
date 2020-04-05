#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
/*
for(i=0; i<n; i++)
	printf("%f\n", probabilities[i]);
 */

const uint64_t zero_and_one_vecs[64] = {0x1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff, 0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,
	                        0xffff,0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff,0x7fffff,0xffffff, 0x1ffffff,
	                        0x3ffffff,0x7ffffff,0xfffffff,0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff, 0x1ffffffff,
	                        0x3ffffffff,0x7ffffffff,0xfffffffff,0x1fffffffff,0x3fffffffff,0x7fffffffff,0xffffffffff,
	                        0x1ffffffffff,0x3ffffffffff,0x7ffffffffff,0xfffffffffff,0x1fffffffffff,0x3fffffffffff,
	                        0x7fffffffffff,0xffffffffffff,0x1ffffffffffff,0x3ffffffffffff,0x7ffffffffffff,0xfffffffffffff,
	                        0x1fffffffffffff,0x3fffffffffffff,0x7fffffffffffff,0xffffffffffffff, 0x1ffffffffffffff,
	                        0x3ffffffffffffff,0x7ffffffffffffff,0xfffffffffffffff,0x1fffffffffffffff,0x3fffffffffffffff,
	                        0x7fffffffffffffff,0xffffffffffffffff};

const float lower_bound = -20;
const float upper_bound = -3.1;


typedef struct individual
{
	int value;
	float probability;
	float loss_function_value;
} individual;

float int_to_float(int x, int bits_num){
	return lower_bound + x*((upper_bound-lower_bound) / (pow(2, bits_num) - 1));
}

void create_population(int n, individual* population, int low, int up){
    int i;
    srand(time(NULL));
    for(i=0; i<n; i++)
    	population[i].value = low + rand() % (up - low + 1);
}

int define_number_of_bits(int accuracy){
	int interval = (upper_bound - lower_bound) * pow(10, accuracy);
	int i = 1;
	while (pow(2, i) - interval < 0){
		i++;
	}
	return i;
}

float f(int x){
	return sin(2 * x) / (x * x);
}

float sum_value_of_all_fitnesses(individual* population, int n){
	float sum = 0;
	for(int i=0; i<n; i++)
	    sum += population[i].loss_function_value;
	return sum;
}

void define_probabilities(individual* population, int n, float sum_of_all_fitnesses){
	for(int i=0; i<n; i++)
		population[i].probability = population[i].loss_function_value / sum_of_all_fitnesses;
	
}


int roulette_wheel_selection(individual* population, int n){
	srand(time(NULL));
	float random_val = rand() / (double) RAND_MAX;
	float offset = 0.0;
	int pick = 0;
	for(int i=0; i<n; i++)
	{
		offset += population[i].probability;
		if (random_val < offset)
		{
			pick = i;
			break;
		}
	}
	return pick;
}


float calculte_loss_function_for_individuals(individual* population, int n, int bits_num){
	/*
	Функция возвращает минимальное значение функции для текущей популяции. В дальнейшем это 
	значение будет использовано для того, чтобы отнормировать (сделать положительными) все значения 
	фитнесс функций. Это необходимо для того, чтобы вычислить вероятности попадения хромосомы в 
	промежуточную популяцию.
	*/
	population[0].loss_function_value = f(int_to_float(population[0].value, bits_num));
	float min_f_value = population[0].loss_function_value;
	for(int i=1; i<n; i++)
	{
		population[i].loss_function_value = f(int_to_float(population[i].value, bits_num));
		if (population[i].loss_function_value < min_f_value)
			min_f_value = population[i].loss_function_value;
	}
	return min_f_value;

}

void normalize_all_f_values(individual* population, int n, float min_f_value){
	for(int i=0; i<n; i++)
		population[i].loss_function_value -= min_f_value;
}

void denormalize_all_f_values(individual* population, int n, float min_f_value){
	for(int i=0; i<n; i++)
		population[i].loss_function_value += min_f_value;
}


void create_intermidiate_population(individual* population,int n, individual* intermidiate_population){
	for(int i=0; i<n; i++)
	{
	   int individual_num = roulette_wheel_selection(population, n);
	   intermidiate_population[i] = population[individual_num];
	}
}


// не проверял
void swap_bits(individual* chromosome_1, individual* chromosome_2,int k){
	int x_1 = chromosome_1->value & zero_and_one_vecs[k];
	int x_2 = chromosome_2->value & zero_and_one_vecs[k];
	chromosome_1->value &= (~zero_and_one_vecs[k]);
	chromosome_1->value |= x_2; 
	chromosome_2->value &= (~zero_and_one_vecs[k]);
	chromosome_2->value |= x_1; 

	// uint64_t x1=vector[x]&cross[z],x2=vector[y]&cross[z];
	// vector[x] &= (~cross[z]);
	// vector[y] &= (~cross[z]);
	// vector[x] |= x2;
	// vector[y] |= x1;
}

void crossingover(individual* intermidiate_population,int n, int bits_num){
	srand(time(NULL));
	float random_val = rand() / (double) RAND_MAX;
	for(int i=0; i<n; i++)
	{
		if (random_val >= 0.5)
		{
			int chromosome_1 = rand() % n;
			int chromosome_2 = rand() % n;
			int position_of_swapping_bits = 1 + (rand() % (bits_num - 2));
			swap_bits(&intermidiate_population[chromosome_1], &intermidiate_population[chromosome_2], position_of_swapping_bits);
		}
	}
}

void mutation(individual* intermidiate_population,int n, int bits_num){
	srand(time(NULL));
	float random_val = rand() / (double) RAND_MAX;
	for(int i=0; i<n; i++)
	{
		if (random_val <= 0.001)
		{
			int position_of_mutation_bit = rand() % (bits_num - 1);
			intermidiate_population[i].value ^= 1 << position_of_mutation_bit;
		}
	}
}

float get_max(individual* population, int n, int bits_num){
	float max = population[0].loss_function_value;
	for(int i=0; i<n; i++)
	{
		if (population[i].loss_function_value>max)
			max = population[i].loss_function_value;
	}
	return max;
}

// void intermidiate_population_to_new_population(individual* population, individual* intermidiate_population, int n, float min_f_value){
// 	// Промежуточная популяция становится новой. Также происходит денормалзация, т.е. от всех 
// 	// значений фитнесс функций вычитается минимальное значение
// 	for(int i=0; i<n; i++)
// 	{
// 		population[i].loss_function_value = intermidiate_population[i].loss_function_value + min_f_value;
// 		population[i].loss_function_value = intermidiate_population[i].loss_function_value + min_f_value;
// 	}
	
// }


int main(){
	int n = 500;
	int bits_num = define_number_of_bits(3);
	int cuts_num = pow(2, bits_num);
	individual *population = (individual*) malloc(n * sizeof(individual));
	individual *intermidiate_population = (individual*) malloc(n * sizeof(individual));
    create_population(n, population, 0, cuts_num);
    for(int j=0; j<30;j++)
    {
    	 create_population(n, population, 0, cuts_num);
	    for(int i=0; i<1000; i++)
	    {
	    	// начало репродукции
			float min_f_value = calculte_loss_function_for_individuals(population, n, bits_num);
			//printf("%f\n", int_to_float(population[5].loss_function_value, bits_num));
			if (min_f_value < 0)
				normalize_all_f_values(population, n, min_f_value);
			float sum = sum_value_of_all_fitnesses(population, n);
			define_probabilities(population, n, sum);
			create_intermidiate_population(population, n, intermidiate_population);

			/*
			Стадия репродукции окончена. Далее следуют стадии кроссинговера
			и мутации 
			*/
			crossingover(intermidiate_population, n, bits_num);
			mutation(intermidiate_population, n, bits_num);
			if (min_f_value > 0)
				min_f_value = 0;
			individual *temp = intermidiate_population;
			intermidiate_population = population;
			population = temp;
			if (min_f_value < 0)
				denormalize_all_f_values(population, n, min_f_value);
		}
    
		float res = get_max(population, n, bits_num);
		printf("%f\n", res);
    }
	//ntermidiate_population_to_new_population(population, intermidiate_population, n, min_f_value);
	//
    //for(int i=0; i<n; i++)
	//    printf("%f\n", probabilities[i]);
	return 0;
}
