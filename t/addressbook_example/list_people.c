// See README.txt for information and build instructions.
#include <stdio.h>
#include <stdlib.h>

#include "addressbook.pb-c.h"

// Main function:  Reads the entire address book from a file and prints all
//   the information inside.
int main() 
{
	FILE * pFile;
	pFile = fopen("people_list2.txt", "rb");

	fseek(pFile, 0, SEEK_END);
	long lFileSize = ftell(pFile);
	rewind(pFile);

	char* buffer = (char*) malloc (sizeof(char)*lFileSize);
	fread (buffer, 1, lFileSize, pFile);

	Tutorial__AddressBook* address_book = tutorial__address_book__unpack (NULL, (size_t)lFileSize, (uint8_t*)buffer);
	printf("Tutorial::AddressBook - total %d people listed.\n", address_book->n_people);
	for (int i = 0; i < address_book->n_people; i++)
	{
		Tutorial__Person** person_list = address_book->people;
		Tutorial__Person* person = &person_list;
		printf("[%d]th person information --\n", i);
		printf(" id = %d\n", person->id);
		printf(" name = %s\n", person->name);
		printf(" email = %s\n", person->email);
		printf(" n_phones = %d\n", person->n_phones);
		break;

	}

  //ListPeople(address_book);


  return 0;
}
