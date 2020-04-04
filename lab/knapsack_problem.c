#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>


const int prob=1000;
const int iter=1000;
const int pop=500;
const int cross_prob=100;
const int mut_prob=150;
const int tourn_size=2;

const uint64_t cross[64] = {0x1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff, 0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff,0x1ffff,0x3ffff,0x7ffff,0xfffff,0x1fffff,0x3fffff,0x7fffff,0xffffff, 0x1ffffff,0x3ffffff,0x7ffffff,0xfffffff,0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff, 0x1ffffffff,0x3ffffffff,0x7ffffffff,0xfffffffff,0x1fffffffff,0x3fffffffff,0x7fffffffff,0xffffffffff, 0x1ffffffffff,0x3ffffffffff,0x7ffffffffff,0xfffffffffff,0x1fffffffffff,0x3fffffffffff,0x7fffffffffff,0xffffffffffff,0x1ffffffffffff,0x3ffffffffffff,0x7ffffffffffff,0xfffffffffffff,0x1fffffffffffff,0x3fffffffffffff,0x7fffffffffffff,0xffffffffffffff, 0x1ffffffffffffff,0x3ffffffffffffff,0x7ffffffffffffff,0xfffffffffffffff,0x1fffffffffffffff,0x3fffffffffffffff,0x7fffffffffffffff,0xffffffffffffffff};

typedef struct element
{
	uint64_t weight;
	uint64_t value;
} element;
//ГЕНЕРАЦИЯ ЗАДАЧИ О РЮКЗАКЕ, ВОЗВРАЩАЕМ ЦЕЛЕВОЙ ВЕС, ЗАПОЛНЯЕМ МАССИВ ПРЕДМЕТОВ
uint64_t knapsack_problem_generator(float density, uint8_t elements_num, element* elements){
	uint64_t max_weight = pow(2, elements_num / density), sum = 0;
	for(int i=0; i<elements_num;i++)
	{
		elements[i].weight = rand() % max_weight;
		sum += elements[i].weight;
		elements[i].value = rand() % 100;
	}
	return (uint64_t)sum / 2;

}
//ВОЗВРАЩАЕТ СУММАРНЫЕ ВЕС ВЫБРАННЫХ ПРЕДМЕТОВ, ЕСЛИ ОН НЕ БОЛЬШЕ МАКСИМАЛЬНОГО
uint64_t fitness_func(element* elements, uint64_t chromo, uint64_t max_sum){
	uint64_t temp=1,sumv=0,sumw=0;
	for(int i=0; i < 64; i++){
		if((temp & chromo)){
			sumv += elements[i].value;
			sumw += elements[i].weight;
			if (sumw > max_sum)
				return 0;
		}
		temp <<= 1;
	}
	return sumv;
}

void read_csv(int line_num, float *density, uint8_t *dim){
	FILE *fp;
	int err=0;
	fp=fopen("in.csv", "r");
	if(fp != NULL){
		for(int i=0; i < line_num+1; i++)
			err=fscanf(fp, "%f,%hhu\n",density,dim);
		fclose(fp);
		if(err == 2)
			printf("READ OK\n");
	}else{
		printf("ERROR READING FILE\n");
	}
}

void write_csv(float density, uint8_t dim, double time_g, double time_d,uint64_t resG, uint64_t resD){
	FILE *fp;
	fp=fopen("out.csv", "a+");
	if(fp != NULL){
		fprintf(fp, "%f;%hhu;%f;%f;%lu;%lu\n",density,dim,time_g,time_d, resG, resD);
		fclose(fp);
	}else{
		printf("ERROR READING FILE\n");
	}
}
//ПО ЧИСЛУ ЭЛЕМЕНТОВ ГЕНЕРИРУЕМ СЛУЧАЙНО ВЕКТОРЫ ИНДИКАТОРЫ МНОЖЕСТВА ПРЕДМЕТОВ
void generate_genes(uint64_t genes[pop],const uint8_t dim){
	uint64_t mod=(1 << dim);
	if (dim==64)
		mod = ~mod+(1 << dim);
    for(int i=0; i < pop; i++)
        genes[i] = rand() % mod;
}

void print_pop(const uint64_t genes[pop]){
    for(int i=0; i < pop;i++)
        printf("%lu\n",genes[i]);
}

int max_func_ind(const uint64_t genes[pop], const int size, element* elements, const uint64_t sum){
	int ind=0;
	uint64_t max_val=0;
	for(int i=0; i < size; i++)
		if(fitness_func(elements,genes[i],sum) > max_val){
			ind=i;
			max_val=fitness_func(elements,genes[i],sum);
		}
	return ind;
}

void reproduct_tournament(uint64_t **vector, const int size, element* elements, const uint64_t sum){
	uint64_t temp[size], new_vector[pop];
	for(int i=0; i<pop;i++){
		for(int j=0; j<size; j++){
			temp[j]=vector[0][rand()%pop];
		}
		new_vector[i]=temp[max_func_ind(temp, size, elements, sum)];
	}
	memcpy(vector[0],new_vector,pop*sizeof(uint64_t));
}

void crossover(uint64_t *vector, const uint8_t dim){
	int x=rand()%pop,y=rand()%pop,z=rand()%dim;
	uint64_t x1=vector[x]&cross[z],x2=vector[y]&cross[z];
	vector[x] &= (~cross[z]);
	vector[y] &= (~cross[z]);
	vector[x] |= x2;
	vector[y] |= x1;
}

void mut(uint64_t *vector, const uint8_t dim){
	int x=rand() % dim,y=rand()%pop;
	vector[y] ^= (1 << x);
}

uint64_t max(uint64_t num1, uint64_t num2)
{
    return (num1 > num2 ) ? num1 : num2;
}

void generate_max_value_matrix(int elements_num, uint64_t knapsack_volume,uint64_t** max_value_matrix,  element* elements){
	for(int k=0; k<=elements_num; k++)
	{
		for(int s=0;s<=knapsack_volume; s++)
		{
			if(k==0 || s==0)
				max_value_matrix[k][s]=0;
			else if (s >= elements[k-1].weight)
				max_value_matrix[k][s] = max(max_value_matrix[k-1][s], max_value_matrix[k-1][ s-elements[k-1].weight] + elements[k-1].value);
			else
				max_value_matrix[k][s] = max_value_matrix[k-1][s];
		}
	}
}

void find_elements_in_knapsack(int elements_num, uint64_t knapsack_volume, uint64_t** max_value_matrix, element* elements, int* answer,int k, int s){
	if (max_value_matrix[k][s] == 0)
		return;
	if (max_value_matrix[k-1][s] == max_value_matrix[k][s])
		find_elements_in_knapsack(elements_num, knapsack_volume, max_value_matrix, elements, answer,k-1,s);
	else {
		find_elements_in_knapsack(elements_num, knapsack_volume,max_value_matrix, elements, answer,k-1,s-elements[k-1].weight);
		answer[k-1] = k;
	}
}

static double diffclock(clock_t clock1,clock_t clock2)
{
    double diffticks=clock1-clock2;
    double diffms=(diffticks)/(CLOCKS_PER_SEC/1000);
    return diffms;
}

void GA(const uint8_t dim, element *pack, const uint64_t sum, uint64_t *res, uint64_t *genes){
	uint64_t sumw=0,sumv=0,temp=1;
	int p;
    srand(clock());
	generate_genes(genes, dim);
	//print_pop(genes);
	for(int i=0; i<iter; i++){
		reproduct_tournament(&genes,tourn_size,pack,sum);
			p=rand() % prob;
			if(p < cross_prob)
				crossover(genes,dim);
			p=rand() % prob;
			if(p < mut_prob)
				mut(genes,dim);
	}
	genes[0]=genes[max_func_ind(genes, dim, pack, sum)];
	//printf("GENETIC SOLUTION\n");
	for(int i=0; i<=dim; i++){
		if (genes[0] & temp){
			//printf("%i\n",i+1);
			sumv += pack[i].value;
			sumw += pack[i].weight;
		}
		else{
			//printf("0\n");
		}
		temp <<= 1;
	}
	*res=sumv;
	//printf("sum_v = %lu\nsum_w = %lu\n",sumv,sumw);
}

void DP(const uint8_t dim, element *pack, const uint64_t sum, uint64_t *res,uint64_t** max_value_matrix, int *answer){
	uint64_t sumw=0, sumv=0;
	generate_max_value_matrix(dim, sum, max_value_matrix, pack);
	memset(answer,0,(dim+1)*sizeof(int));
	find_elements_in_knapsack(dim, sum, max_value_matrix,pack, answer, dim, sum);
	//printf("DYNAMIC SOLUTION\n");
	for (int i = 0; i<=dim; i++){
		//printf("%d\n", answer[i]);
		if(answer[i] != 0){
			sumv += pack[i].value;
			sumw += pack[i].weight;
		}
	}
	*res = sumv;
	//printf("sum_v = %lu\nsum_w = %lu\n",sumv,sumw);
}


int main(void) {
  clock_t start, end;
	FILE *fp;
	float density;
	uint8_t dim;
	uint64_t sum, resG, resD;
	element* pack;
	fp=fopen("out.csv", "w");
	fclose(fp);
	int *answer;
	uint64_t **max_value_matrix;
	srand(clock());
	uint64_t *genes = malloc(sizeof(uint64_t)*pop);
  	for(int k=0; k<7; k++)
	{
		read_csv(k, &density, &dim);
		pack=malloc(sizeof(element)*dim);
		max_value_matrix = malloc((dim+1)*sizeof*max_value_matrix);
		answer=(int *)malloc(sizeof(int)*(dim+1));
		for(int j=0; j<100;j++)
		{
			sum=knapsack_problem_generator(density, dim, pack);
			for(int i = 0; i<=dim; i++)
				max_value_matrix[i] = malloc((sum+1) * sizeof*max_value_matrix[i]);
			//for(int i=0;i<dim;i++)
			//	printf("w = %lu v = %lu\n",pack[i].weight,pack[i].value);
			//printf("max_w = %lu\n",sum);
			start =clock();
		    GA(dim,pack,sum,&resG,genes);
			end = clock();
			double time_g = diffclock(end, start);;
		 	start = clock();
			DP(dim,pack,sum,&resD,max_value_matrix,answer);
			end = clock();
			double time_d = diffclock(end, start);
			write_csv(density, dim, time_g, time_d,resG,resD);
			for (int i = 0; i<=dim; i++) 
				free(max_value_matrix[i]);
		}
		free(pack);
    	free(max_value_matrix);
    	free(answer);
	}
	free(genes);
	return 0;
}