/**
 * @file    main.c
 * @brief   tool for automatic test plan generation
 * @author  Pavel Marusyk (p.marusik@samsung.com)
 * @version 1.0
 * @date    Created March 2, 2013 10:23
 * @par     In Samsung Ukraine R&D Center (SRK) under a contract between
 * @par     LLC "Samsung Electronics Ukraine Company" (Kiev, Ukraine) and "Samsung Elecrtronics Co", Ltd (Seoul, Republic of Korea)
 * @par     Copyright: (c) Samsung Electronics Co, Ltd 2013. All rights reserved.
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <sys/mman.h>

#define PAGE_SIZE 0x1000
#define PAGEMAP_ENTRY 8
#define GET_BIT(X,Y) (X & ((uint64_t)1<<Y)) >> Y
#define GET_PFN(X) X & 0x007FFFFFFFFFFFFF

uint8_t test_values[PAGE_SIZE] __attribute__((aligned(0x1000)));
//uint8_t *test_values  __attribute__((aligned(0x1000)));

const int __endian_bit = 1;
#define is_bigendian() ( (*(char*)&__endian_bit) == 0 )

/**
  * @fn     void print_buffer ( uint8_t *buffer, uint32_t buffer_len, uint32_t columns_count, uint32_t rows_count )
  * @brief  prints content of buffer on screen
  * @param  [in] *out_text          - text to be printed before data, NULL if no text to be printed
  * @param  [in] *buffer            - buffer to be printed out
  * @param  [in]  buffer_len        - lenght of data to be outputed
  * @param  [in]  columns_count     - how many bytes must printed, if <= 0 - then by default 8
  * @param  [in]  rows_count        - how many should be printed, if <= 0 - no limited rows will be used
  * @param  [in] *numbers_separator - numbers separator, if NULL no separations is used
  */
void print_buffer( char    *out_text,
                   uint8_t *buffer,
                   uint32_t buffer_len,
                   uint32_t columns_count,
                   uint32_t rows_count,
                   char    *numbers_separator)
{
    #define OUTPUT_TEXT_SIZE 256

    uint32_t i;
    uint32_t is_new_line = 0; // 0 = new line;
    char     output_text[OUTPUT_TEXT_SIZE] = {0};

    // check if rows_count isn't set
    rows_count = ( rows_count > 0x7FFFFF || rows_count == 0 ) ? 0x7FFFFF : rows_count;

    // check if columns_count isn't set
    if ( columns_count > 0x7FFFFF || columns_count == 0 )
    {
        if ( buffer_len > 16 )
        {
            columns_count = 16;
        }
        else
        {
            columns_count = buffer_len;
        }
    }

    // default String
    snprintf(output_text, OUTPUT_TEXT_SIZE, "%s[", (out_text == NULL) ? "" : out_text);

    // main cycle
    for ( i = 0; i < buffer_len; i++)
    {
        // main output
        printf("%s%02x", (is_new_line == 0) ? output_text : "" , buffer[i]);
        //printf("%s%02x", (i == 0) ? output_text : "" , buffer[i]);

        // if table size not enough for output information, out put is break
        if ( i >= columns_count * rows_count - 3 - 1 )
        {
            printf("......");
            break;
        }

        // check is end of lines
        if ( i != buffer_len - 1 )
        {
            is_new_line = ((i+1) % columns_count);
            if( is_new_line == 0 )
            {
                printf("]\n");
                if ( out_text != NULL )
                {
                    memset(output_text, ' ', strlen(out_text));
                    //printf("flag !!!! |%s| \n", output_text);
                }
            }
            else
            {
                // print number separator
                printf("%s", ( numbers_separator == NULL ) ? "" : numbers_separator);
            }
        }
    }
    printf("]\n");
}

uint64_t read_pagemap(char * path_buf, uint64_t virt_addr)
{
    uint64_t file_offset;
    int     fd;
    __off_t status, paddr;
    size_t  bytes_read;

    // translate viratual address to VFR
    file_offset = ((virt_addr) >> 12) * PAGEMAP_ENTRY;

    printf(" [read_pagemap] - VFN and offset (0x%lx) (0x%lx)\n", file_offset / 8, file_offset);

    do
    {
        // Opennong fd of /proc/[pid]/pagemap
        fd = open(path_buf, O_CREAT);
        if( fd == -1 )
        {
          printf("\n [read_pagemap] - file open Error (0x%x) - %s. \n\n", errno, strerror( errno ));
          break;
        }

        // Seek in File
        status = lseek(fd, file_offset, SEEK_SET);
        if ( status != file_offset )
        {
            printf("\n [read_pagemap] - lseek operation Error (0x%x) - %s. \n\n", errno, strerror( errno ));
        }

        // Read pagemmem File
        bytes_read = read(fd, &paddr, PAGEMAP_ENTRY);
        if ( bytes_read != PAGEMAP_ENTRY )
        {
            printf("\n  [read_pagemap] - Read Operation Error (0x%x) - %s. \n\n", errno, strerror( errno ));
        }

        close(fd);

    } while(0);

    //
    printf(" [read_pagemap] - read operation value : 0x%lx\n", paddr);

    if(GET_BIT(paddr, 62))
        printf(" >>>>>>>>>>>>>>>>>: Page swapped\n");

    // GET PFN
    if ( GET_BIT(paddr, 63) )
    {
        paddr = (GET_PFN(paddr)) << 12;
        printf(" [read_pagemap] - Physical address     : 0x%lx\n", paddr);

        return paddr;
    }
    else
        printf(" >>>>>>>>>>>>>>>>>: Page not present\n");

    return 0;
}


int main(int argc, char ** argv)
{
    int  fd;


    fd = open("/dev/mem", O_RDONLY | O_SYNC);

    int *values = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, (off_t)0);

    perror("mmap x");
    printf( "\n\n %p \n\n", values);

    close(fd);

    /*

    uint64_t *virt_addr;
    uint64_t  phys_addr = 0;

    char   pagemap_filename[255];

    //uint8_t test_values[PAGE_SIZE] __attribute__((aligned(0x1000)));

    uint32_t sizeof_test_array = sizeof(test_values);
    memset(test_values, 0x01, PAGE_SIZE);

    #if __x86_64__
        printf("\n >>>>  Platfrom x64  <<<<<<\n");
    #endif

    // checking of input paramters
    if ( argc != 2 )
    {
        printf(" PID                :  %d\n", getpid());
        printf("\n");

        //snprintf(pagemap_filename, 255, "/proc/%d/pagemap", getpid());
        snprintf(pagemap_filename, 255, "/proc/self/pagemap");

        // Virt to phys
        phys_addr = read_pagemap(pagemap_filename, (uint64_t)test_values);

        print_buffer( " real values      : ", test_values, sizeof_test_array, 32, 1, " ");

        sleep(11);

        print_buffer( " mmaped values    : ", test_values, sizeof_test_array, 32, 1, " ");

        printf("\n\n");
    }
    else
    {
        phys_addr = strtol(argv[1], NULL, 16);

        printf("   phys_addr = 0x%lx \n ", phys_addr);
        // mmap
        do
        {
            // opening device
            fd = open("/dev/mem", O_RDONLY | O_SYNC);
            if (fd <= 0)
            {
                printf("Can't open device file: %s with error: %d\n", "/dev/mem", errno);
                break;
            }
            printf(" /dev/mem opened succesfully \n");

            // Mapping
            do
            {
                errno = 0;
                virt_addr = mmap(NULL, PAGE_SIZE, PROT_READ, MAP_PRIVATE, fd, phys_addr);

                if ( errno != 0 || virt_addr == MAP_FAILED)
                {
                    printf("\n - MMAP Error (0x%x) - %s. \n\n", errno, strerror( errno ));
                    break;
                }
                printf(" Mapped address : virt_addr = %p\n", virt_addr);

                // set values
                memset(virt_addr, 0x02, PAGE_SIZE);

                munmap(virt_addr, PAGE_SIZE);

            } while (0);

            close(fd);

        } while (0);
    }
*/
    return 0;

}

