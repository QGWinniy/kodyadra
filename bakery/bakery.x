program BAKERY_PROG {
    version BAKERY_VER {
        int GET_TICKET(int client_id) = 1;
        int GET_PROCESSING_TIME(int ticket_number) = 2;
    } = 1;
} = 0x30000001;
