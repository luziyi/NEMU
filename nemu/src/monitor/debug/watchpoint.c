#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool()
{
    int i;
    for (i = 0; i < NR_WP; i++)
    {
        wp_pool[i].NO = i;
        wp_pool[i].next = &wp_pool[i + 1];
    }
    wp_pool[NR_WP - 1].next = NULL;

    head = NULL;
    free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp()
{
    WP *temp;
    temp = free_;
    free_ = free_->next;
    temp->next = NULL;
    if (head == NULL)
    {
        head = temp;
    }
    else
    {
        WP *temp2;
        temp2 = head;
        while (temp2->next != NULL)
        {
            temp2 = temp2->next;
        }
        temp2->next = temp;
    }
    return temp;
}

void free_wp(WP *wp)
{
    if (wp == NULL)
    {
        assert(0);
    }
    if (wp == head)
    {
        head = head->next;
    }
    else
    {
        WP *temp = head;
        while (temp != NULL && temp->next != wp)
        {
            temp = temp->next;
        }
        temp->next = temp->next->next;
    }
    wp->next = free_;
    free_ = wp;
    wp->result = 0;
    wp->expr[0] = '\0';
}

void print_wp()
{
    WP *temp = head;
    if (temp == NULL)
    {
        printf("No watchpoints\n");
    }
    while (temp != NULL)
    {
        printf("Watch point %d: %s\n", temp->NO, temp->expr);
        temp = temp->next;
    }
}

WP *delete_wp(int p, bool *key)
{
    WP *temp = head;
    while (temp != NULL && temp->NO != p)
    {
        temp = temp->next;
    }
    if (temp == NULL)
    {
        *key = false;
    }
    return temp;
}