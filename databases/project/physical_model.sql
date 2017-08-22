-- Autor: Łukasz Siudek
-- Indeks: 283493

/* typy danych */
CREATE DOMAIN usr_type AS varchar(12)
  CHECK (VALUE IN ('user', 'organizer'));

CREATE DOMAIN talk_type AS varchar(12)
  CHECK (VALUE IN ('accepted', 'proposed', 'rejected'));

/* tabele */
CREATE TABLE usr (
  login varchar(16) NOT NULL,
  password varchar(16) NOT NULL,
  status usr_type NOT NULL DEFAULT 'user',
  PRIMARY KEY(login)
);

CREATE TABLE event (
  name varchar(64) NOT NULL,
  start_timestamp timestamp NOT NULL,
  end_timestamp timestamp NOT NULL,
  PRIMARY KEY(name),
  CHECK (end_timestamp >= start_timestamp)
);

CREATE TABLE talk (
  talk varchar(64) NOT NULL,
  title varchar(64) NOT NULL,
  start_timestamp timestamp NOT NULL,
  room integer,
  status talk_type NOT NULL DEFAULT 'proposed',
  eventname varchar(64),
  speakerlogin varchar(16) NOT NULL,
  accept_timestamp timestamp,
  PRIMARY KEY(talk),
  FOREIGN KEY(eventname) REFERENCES event(name) DEFERRABLE,
  FOREIGN KEY(speakerlogin) REFERENCES usr(login) DEFERRABLE
);

CREATE TABLE evaluation (
  id SERIAL NOT NULL,
  rating integer NOT NULL,
  login varchar(16) NOT NULL,
  talk varchar(64) NOT NULL,
  UNIQUE (login, talk),
  FOREIGN KEY(login) REFERENCES usr(login) DEFERRABLE,
  FOREIGN KEY(talk) REFERENCES talk(talk) DEFERRABLE,
  CHECK(rating >= 0 AND rating <= 10)
);

CREATE TABLE friends (
  date_concluded timestamp,
  login varchar(16) NOT NULL,
  friendlogin varchar(16) NOT NULL,
  UNIQUE(login, friendlogin),
  FOREIGN KEY(login) REFERENCES usr(login) DEFERRABLE,
  FOREIGN KEY(friendlogin) REFERENCES usr(login) DEFERRABLE
);

CREATE TABLE attendance (
  login varchar(16) NOT NULL,
  talk varchar(64) NOT NULL,
  UNIQUE(login, talk),
  FOREIGN KEY(login) REFERENCES usr(login) DEFERRABLE,
  FOREIGN KEY(talk) REFERENCES talk(talk) DEFERRABLE
);

CREATE TABLE registered (
  login varchar(16) NOT NULL,
  eventname varchar(64) NOT NULL,
  UNIQUE(login, eventname),
  FOREIGN KEY(login) REFERENCES usr(login) DEFERRABLE,
  FOREIGN KEY(eventname) REFERENCES event(name) DEFERRABLE
);

/* perspektywy */
CREATE VIEW made_friends AS
  SELECT * FROM friends
  WHERE date_concluded IS NOT NULL;

CREATE VIEW accepted_talks AS
  SELECT * FROM talk
  WHERE status = 'accepted';

CREATE VIEW proposed_talks AS
  SELECT * FROM talk
  WHERE status = 'proposed';

CREATE VIEW rejected_talks AS
  SELECT * FROM talk
  WHERE status = 'rejected';

/* wyzwalacze */
-- jesli po wyslaniu zaproszenia mamy znajomosc w dwie strony,
-- to zapamietujemy date zawarcia znajomosci
CREATE OR REPLACE FUNCTION friends_update() RETURNS TRIGGER AS
$X$
BEGIN
  UPDATE friends
  SET date_concluded = now()
  WHERE NEW.login = friendlogin
    AND NEW.friendlogin = login
    AND date_concluded IS NULL;
  IF (FOUND) THEN
    NEW.date_concluded = now();
    RETURN NEW;
  END IF;
  RETURN NEW;
END;
$X$
LANGUAGE plpgsql;

CREATE TRIGGER on_insert_to_friends BEFORE INSERT ON friends
FOR EACH ROW EXECUTE PROCEDURE friends_update();

CREATE OR REPLACE FUNCTION score(login varchar(16), talk varchar(64))
RETURNS real AS
$X$
DECLARE
  avg_rating real;
  talk_attendance integer;
  num_friends integer;
  result real;
BEGIN
  SELECT avg(e.rating) INTO avg_rating FROM evaluation e
  WHERE e.talk=$2;

  SELECT count(a.login) INTO talk_attendance FROM attendance a
  WHERE a.talk=$2;

  SELECT count(a.login) INTO num_friends FROM attendance a
  JOIN made_friends f ON (a.login=f.login)
  WHERE a.talk=$2 AND f.friendlogin=$1;

  result := (num_friends * 10 + talk_attendance)::real * avg_rating;
  IF (result IS NULL)
    THEN RETURN 0;
  END IF;
  RETURN result;
END;
$X$
LANGUAGE plpgsql;
