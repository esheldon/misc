alter table contacts add contact_email text;
alter table contacts add contact_smail text;
alter table contacts add contact_appeal text;

alter table contacts add spouse text;

update contacts set contact_email = 'y';
update contacts set contact_smail = 'y';
update contacts set contact_appeal = 'n';

-- marked as contributors and not a: no appeal and not zz
update contacts set contact_appeal='y' where gift like 'c' and contact not like 'a' and contact not like 'zz';
