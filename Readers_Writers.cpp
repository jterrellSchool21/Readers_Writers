#include <omp.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include <time.h>
#include <locale.h>
#include <windows.h>

FILE* stream;

void	printTimeStamp()
{
	SYSTEMTIME	timeStamp;
	GetLocalTime(&timeStamp);
	printf("%02d:%02d:%02d:%03d\n", timeStamp.wHour, timeStamp.wMinute, timeStamp.wSecond, timeStamp.wMilliseconds);
	fprintf(stream, "%02d:%02d:%02d:%03d", timeStamp.wHour, timeStamp.wMinute, timeStamp.wSecond, timeStamp.wMilliseconds);
}

void	writer(int& storage, int totalWriters, int& activeReaders, omp_lock_t& writeLock, bool& readLock)
{
#pragma omp parallel num_treads(totalWriters)
	{
		bool	flag;
		int		writerNumber = omp_get_thread_num();

		while (true)
		{
			flag = true;
			Sleep(rand() * writerNumber % 15000 + 3000);

#pragma omp critical
			{
				printTimeStamp();
				printf("Писатель %d пишет в хранилище\n", writerNumber);
				fprintf(stream, "Писатель %d пишет в хранилище\n", writerNumber);
			}
			if (!omp_test_lock(&writeLock))
			{
#pragma omp critical
				{
					printf("Другой писатель обратился к хранилищу раньше\n");
					printf("Писатель %d ожидает\n", writerNumber);
					fprintf(stream, "Другой писатель обратился к хранилищу раньше\n");
					fprintf(stream, "Писатель %d ожидает\n", writerNumber);
				}
				omp_set_lock(&writeLock);
			}
			readLock = true;
			while (activeReaders != 0)
			{
				if (!flag)
				{
#pragma omp critical
					{
						printf("Писатель %d ожидает, пока другие активные читатели закончат работу с хранилищем\n", writerNumber);
						fprintf(stream, "Писатель %d ожидает, пока другие активные читатели закончат работу с хранилищем\n", writerNumber);
					}
				}
				flag = true;
				Sleep(100);
			}
#pragma omp critical
			{
				printTimeStamp();
				printf("Писатель %d получил доступ к хранилищу\n", writerNumber);
				fprintf(stream, "Писатель %d получил доступ к хранилищу\n", writerNumber);
			}
			Sleep(3000);
			storage = rand() + writerNumber;
#pragma omp critical
			{
				printTimeStamp();
				printf("Писатель %d записал в хранилище сообщение %d и закончил работу\n", writerNumber, storage);
				fprintf(stream, "Писатель %d записал в хранилище сообщение %d и закончил работу\n", writerNumber, storage);

			}
			readLock = false;
			omp_unset_lock(&writeLock);
		}
	}
}

void	reader(int& storage, int totalReaders, int& activeReaders, bool& readLock)
{
#pragma omp parallel num_treads(totalReaders)
	{
		bool	flag;
		int		readerNumber = omp_get_thread_num();

		while (true)
		{
			flag = true;
			Sleep(rand() * readerNumber % 12000 + 3000);

#pragma omp critical
			{
				printTimeStamp();
				printf("Читатель %d обратился к хранилищу\n", readerNumber);
				fprintf(stream, "Читатель %d обратился к хранилищу\n", readerNumber);
			}
			while (readLock == true)
			{
				if (!flag)
				{
#pragma omp critical
					{
						printf("Доступ к хранилищу заблокирован, читатель %d ожидает\n", readerNumber);
						fprintf(stream, "Доступ к хранилищу заблокирован, читатель %d ожидает\n", readerNumber);
					}
				}
				flag = true;
				Sleep(100);
			}
#pragma omp critical
			{
				printTimeStamp();
				printf("Читатель %d получил доступ к хранилищу\n", readerNumber);
				fprintf(stream, "Читатель %d получил доступ к хранилищу\n", readerNumber);
			}
#pragma omp atomic
			activeReaders++;
			Sleep(3000);
#pragma omp critical
			{
				printTimeStamp();
				printf("Читатель %d прочитал из хранилища сообщение %d и закончил работу\n", readerNumber, storage);
				fprintf(stream, "Читатель %d прочитал из хранилища сообщение %d и закончил работу\n", readerNumber, storage);
			}
#pragma omp atomic
			activeReaders--;
		}
	}
}

int		main(void)
{
	setlocale(LC_ALL, "Russian");
	srand(time(NULL));

	int		storage = 0;
	int		totalWriters = 0;
	int		totalReaders = 0;
	int		activeReaders = 0;

	omp_lock_t	writeLock;
	bool	readLock = false;

	fopen_s(&stream, "log.txt", "w");

	omp_init_lock(&writeLock);
	omp_set_nested(true);

	std::cout << "Введите количество писателей и читателей" << std::endl;
	std::cin >> totalWriters;
	std::cin >> totalReaders;

#pragma omp parallel sections
	{
#pragma omp section
		{
			writer(storage, totalWriters, activeReaders, writeLock, readLock);
		}
#pragma omp section
		{
			reader(storage, totalReaders, activeReaders, readLock);
		}
	}
	return (0);
}