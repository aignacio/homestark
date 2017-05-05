//init_broker();

etimer_set(&time_poll, CLOCK_SECOND);
rpl_dag_t *dag;
uip_ds6_route_t *r;
// struct rpl_dag {
//   uip_ipaddr_t dag_id;
//   rpl_rank_t min_rank; /* should be reset per DAG iteration! */
//   uint8_t version;
//   uint8_t grounded;
//   uint8_t preference;
//   uint8_t used;
//   /* live data for the DAG */
//   uint8_t joined;
//   rpl_parent_t *preferred_parent;
//   rpl_rank_t rank;
//   struct rpl_instance *instance;
//   rpl_prefix_t prefix_info;
//   uint32_t lifetime;
// };
while(1) {
    PROCESS_WAIT_EVENT();

    /* Let's suppose we have only one instance */
    dag = rpl_get_any_dag();
    if(dag->preferred_parent != NULL) {
      debug_os("Preferred parent: ");
      ghgh(rpl_get_parent_ipaddr(dag->preferred_parent));
      debug_os("\n");
    }
    for(r = uip_ds6_route_head();
        r != NULL;
        r = uip_ds6_route_next(r)) {
      PRINT6ADDR(&r->ipaddr);
    }
    // rpl_repair_root(dag->instance-);
    // debug_os("RETORNO PARENTE:%d",rpl_parent_is_fresh(dag->preferred_parent));
    //rpl_print_neighbor_list();
    // debug_os("Execucao[%d]",tick_process++);\
    // sprintf(pub_test,"Execucao %d",tick_process);
    // mqtt_sn_pub(topic_hw,pub_test,true,0);
    // //mqtt_sn_pub("/topic_6",pub_test,true,0);
    // //mqtt_sn_pub("/topic_5",pub_test,true,0);
    // //mqtt_sn_pub("/topic_4",pub_test,true,0);
    //
    // //mqtt_sn_check_queue();
    // //print_g_topics();
    // //debug_os("Estado do MQTT:%s",mqtt_sn_check_status_string());
    if (etimer_expired(&time_poll))
      etimer_reset(&time_poll);
}
