
resp_con_t send_ping(void){
    debug_snmp("Enviando pacote de @PING");
    ping_req_t packet;

    // uip_ipaddr_t *node_address;
    // node_address = rpl_get_parent_ipaddr(dag->preferred_parent);
    rpl_dag_t *dag;
    dag = rpl_get_any_dag();
    rpl_parent_t *p = nbr_table_head(rpl_parents);
    rpl_instance_t *default_instance;
    default_instance = rpl_get_default_instance();
    while(p != NULL)
      if (p == default_instance->current_dag->preferred_parent) {
        sprintf(packet.message,"No:[%3u]",rpl_get_parent_ipaddr(p)->u8[15]);
        debug_snmp("Endereco do NO:%3u",rpl_get_parent_ipaddr(p)->u8[15]);
        break;
      }
      else
        p = nbr_table_next(rpl_parents, p);

    debug_snmp("RETORNO PARENTE:%d\n",rpl_parent_is_fresh(dag->preferred_parent));
    rpl_print_neighbor_list();
    packet.length = strlen(packet.message);
    simple_udp_send(&nms_con.udp_con,&packet.message, packet.length);
    return SUCCESS_CON;
}
