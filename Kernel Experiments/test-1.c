#include <linux/module.h>	
#include <linux/kernel.h>
#include <linux/pid.h>	
#include <linux/moduleparam.h>         
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <linux/sched/task.h> // SIKINTI ÇIAKRIYOR 4.10'DA DÜZELT

static int processid = 0;

module_param(processid, int, 0);


//static void print_pgd(struct mm_struct *mm,long unsigned vm_start);

static int is_stack(struct vm_area_struct *vma) //SOURCE LINUX KERNEL task_mmu.c
{
	/*
	 * We make no effort to guess what a given thread considers to be
	 * its "stack".  It's not even well-defined for programs written
	 * languages like Go.
	 */
	return vma->vm_start <= vma->vm_mm->start_stack &&
		vma->vm_end >= vma->vm_mm->start_stack;
	}





static void print_task(struct task_struct *task){

	struct mm_struct *mm = task->mm;

	struct vm_area_struct *vma = mm->mmap;

	int total_vma_used = 0;

	while(vma != NULL){

		//STACK

		if(is_stack(vma)){

			printk(KERN_INFO "START--> 0x%016lx ~ END--> 0x%016lx, SIZE--> %luB  ~~STACK\n", vma->vm_start, vma->vm_end, (vma->vm_end - vma->vm_start)/8);

			vma = vma->vm_next;

			total_vma_used++;

			continue;

		}

		//END STACK

		//CODE

		else if(vma->vm_start <= mm->end_code && vma->vm_end >= mm->start_code){

			printk(KERN_INFO "START--> 0x%016lx ~ END--> 0x%016lx, SIZE--> %lu  ~~CODE\n", vma->vm_start, vma->vm_end, (vma->vm_end - vma->vm_start));

		}	

		//

		//DATA

		else if(vma->vm_start <= mm->end_data && vma->vm_end >= mm->start_data){

			printk(KERN_INFO "START--> 0x%016lx ~ END--> 0x%016lx, SIZE--> %lu  ~~DATA\n", vma->vm_start, vma->vm_end, (vma->vm_end - vma->vm_start));

		}

		//END DATA

		//HEAP

		else if (vma->vm_start <= mm->brk && vma->vm_end >= mm->start_brk){

			printk(KERN_INFO "START--> 0x%016lx ~ END--> 0x%016lx, SIZE--> %lu  ~~HEAP\n", vma->vm_start, vma->vm_end, (vma->vm_end - vma->vm_start)); 		

		}

		//END HEAP
		else{
			printk(KERN_INFO "START--> 0x%016lx ~ END--> 0x%016lx, SIZE--> %lu\n", vma->vm_start, vma->vm_end, (vma->vm_end - vma->vm_start)); 
		}

	total_vma_used++;

	vma = vma->vm_next;

	}



	printk(KERN_INFO "START--> 0x%016lx ~ END--> 0x%016lx, SIZE--> %lu ~~ MAIN ARGS\n", mm->arg_start, mm->arg_end, mm->arg_end - mm->arg_start); // START AND END OF THE MAIN

	printk(KERN_INFO "START--> 0x%016lx ~ END--> 0x%016lx, SIZE--> %lu ~~ ENVIRONMENT VARIABLS\n", mm->env_start, mm->env_end, mm->env_end - mm->env_start); // START AND END OF THE ENVIRONMENT

	printk(KERN_INFO "%lu  ~~RSS\n", get_mm_rss(mm)); // RSS

	printk(KERN_INFO "%lu ~~TOTAL PAGES MAPPED\n", mm->total_vm); // TOTAL VM

	printk(KERN_INFO "%d ~~TOTAL VM_AREA_STRUCTS USED\n", total_vma_used);

}

// given the memory descriptor and the begin address of virtual memory, prints out the top level page table
/*
static void print_pgd(struct mm_struct *mm, long unsigned vm_start) {
	long unsigned current_addr = vm_start;
	long unsigned inc = 1LU << 39;

	pgd_t *pgd; 
	int i = 0; 
	printk(KERN_ERR "TOP LEVEL PAGE TABLE ENTRIES:\n");
	for (; i < (1<<9); ++i) {
		// pgd_offset() takes an address and the mm_struct for the process 
		// and returns the PGD entry that covers the requested address.
		pgd = pgd_offset(mm, current_addr);
		if (pgd_none(*pgd) || pgd_bad(*pgd)) {
			printk(KERN_ERR "ENTRY %d, PGD: %lu - DOES NOT EXIST OR BAD\n", i, pgd->pgd);
			break;
		}
		else {
			printk(KERN_INFO "ENTRY %d, PGD: %lu", i, pgd->pgd);
		}
		current_addr += inc;
	}
} 
*/
static int __init test_init(void) {
	struct task_struct *task;
	task = get_pid_task(find_vpid(processid), PIDTYPE_PID);

	if (task == NULL){
		printk(KERN_INFO "\nGIVEN PID %d WAS NOT FOUND, TERMINATING\n", processid);
		return 0;
	}

	if (task->pid == processid){
		printk(KERN_INFO "-----------------------------------------------");
		printk(KERN_INFO "\nGiven pid %d is found.\n",task->pid);
	}

	print_task(task);
	return 0;
}

static void __exit test_exit(void) {
	printk(KERN_INFO "Goodbye world 1.\n");
	printk(KERN_INFO "-----------------------------------------------");
}

module_init(test_init); // INIT MACRO
module_exit(test_exit); // EXIT MACTO
MODULE_LICENSE("GPL");
