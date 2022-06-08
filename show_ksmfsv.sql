connect / as sysdba
set feedback off
set heading off
set pagesize 0
set linesize 200
spool ksmfsv.area
select VERSION||','||KSMFSNAM||','||KSMFSTYP||',0x'|| KSMFSADR||','|| KSMFSSIZ from x$ksmfsv, v$instance order by KSMFSADR;
spool off
quit
