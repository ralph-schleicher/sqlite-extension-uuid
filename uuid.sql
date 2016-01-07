select load_extension('./sqlite3ext_uuid', 'sqlite3ext_uuid');

select uuid(x'70c219c7c69b40988a59d17fb9a688b6') = '70c219c7-c69b-4098-8a59-d17fb9a688b6';
select xuuid('70c219c7-c69b-4098-8a59-d17fb9a688b6') = x'70c219c7c69b40988a59d17fb9a688b6';
select uuid(xuuid('70c219c7-c69b-4098-8a59-d17fb9a688b6')) = '70c219c7-c69b-4098-8a59-d17fb9a688b6';
select xuuid(uuid(x'70c219c7c69b40988a59d17fb9a688b6')) = x'70c219c7c69b40988a59d17fb9a688b6';

create table t1
(
  uuid char(36) primary key not null default ( uuid(uuidgen()) ),
  tag text not null
);

insert into t1 (tag) values ('foo'), ('bar');
select uuid, tag from t1;

create table t2
(
  uuid blob primary key not null default ( uuidgen() ),
  tag text not null
);

insert into t2 (tag) values ('baz'), ('hack');
select uuid(uuid), tag from t2;
