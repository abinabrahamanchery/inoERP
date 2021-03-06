<?php

 class am_wo_work_bench_line extends am_wo_routing_line {

  public function _before_save() {
//   pa($this);
   $existing_data_array = ['am_wo_header_id', 'routing_sequence', 'am_wo_routing_line_id', 'file_id_values'];
   $this->_save_extra_field($existing_data_array);
   return -1;
  }

  public function _save_extra_field($existing_data_array) {
   global $dbc;
   foreach ($_POST as $field_name => $val_a) {
    if (in_array($field_name, $existing_data_array)) {
     continue;
    }
    $class = 'extra_field_' . $field_name;
    $key_column = $class . '_id';
    $value_column = $class . '_value';
    $$class = new stdClass();
    $extra_fields = view::find_columns_of_table($class);
//    pa($extra_fields);
    //check if record already exists
    $checkSql = " SELECT * FROM $class WHERE reference_type = 'table' AND reference_key_name='am_wo_routing_line' "
      . "  AND reference_key_value='{$this->am_wo_routing_line_id}' LIMIT 1";

    $checkResult = self::find_by_sql($checkSql);

    //if $checkResult is not empty update the existing data else create record
    if (!empty($checkResult)) {
     $key_column_value = $checkResult[0]->$key_column;
     if (empty($_POST['file_id_values'][0]) && ($checkResult[0]->$value_column == $_POST[$field_name][0])) {
      continue;
     } else if (!empty($_POST['file_id_values'][0]) && ($checkResult[0]->$value_column == $_POST['file_id_values'][0])) {
      continue;
     }
     //Update the existing content
     $field_stmt = '';
     $field_stmt .= " $value_column = :$value_column ,";
     $field_stmt .= ' last_update_date = :last_update_date ,';
     $field_stmt .= ' last_update_by = :last_update_by ';

//write SQL
     $sql = " UPDATE ";
     $sql.= $class;
     $sql .= " SET ";
     $sql.= $field_stmt;
     $sql.= " WHERE $key_column = '{$key_column_value}' ";
     $stmt = $dbc->connection->prepare(" $sql ");
     //Bind parameters
     if (empty($_POST['file_id_values'][0])) {
      $stmt->bindParam(":$value_column", $_POST[$field_name][0]);
     } else {
      $stmt->bindParam(":$value_column", $_POST['file_id_values'][0]);
     }
     $stmt->bindvalue(':last_update_date', current_time(), PDO::PARAM_INT);
     $stmt->bindvalue(':last_update_by', $_SESSION['username'], PDO::PARAM_INT);
     try {
      $stmt->execute();
      echo "<br>Record is Successfully Saved in $class";
     } catch (PDOException $e) {
      $dbc->rollback = 1;
      echo "<br> $sql <br>";
      echo "<br>Record couldnt be saved. Error @am_wo_work_bench_line @@ Line " . __LINE__ . $e->getMessage() . "<br/>";
     }
    } else {
     $field_colums = '';
     foreach ($extra_fields as $key => $value) {
      $field_colums .= $value . ', ';
     }
     $field_colums = rtrim($field_colums, ', ');

     $field_values = '';
     foreach ($extra_fields as $key1 => $value1) {
      $field_values .= ':' . $value1 . ', ';
     }
     $field_values = rtrim($field_values, ', ');
//write SQL
     $sql = " INSERT INTO ";
     $sql.= $class;
     $sql.= ' ( ';
     $sql.= $field_colums;
     $sql.= ' ) ';
     $sql.= 'VALUES';
     $sql.= ' ( ';
     $sql.= $field_values;
     $sql.= ' ) ';
//		print_r($_POST);
     $stmt = $dbc->connection->prepare(" $sql ");
     //Bind parameters
     foreach ($extra_fields as $key => $value) {
      if (!empty($_POST[$value][0])) {
       $stmt->bindParam(":$value", $_POST[$value][0]);
      } else {
       $stmt->bindvalue(":$value", null, PDO::PARAM_INT);
      }
     }
     $stmt->bindvalue(':reference_type', 'table', PDO::PARAM_INT);
     $stmt->bindvalue(':reference_key_name', 'am_wo_routing_line', PDO::PARAM_INT);
     $stmt->bindParam(':reference_key_value', $_POST['am_wo_routing_line_id'][0]);
     if (empty($_POST['file_id_values'][0])) {
      $stmt->bindParam(":$value_column", $_POST[$field_name][0]);
     } else {
      $stmt->bindParam(":$value_column", $_POST['file_id_values'][0]);
     }
     $stmt->bindvalue(':creation_date', current_time(), PDO::PARAM_INT);
     $stmt->bindvalue(':created_by', $_SESSION['username'], PDO::PARAM_INT);
     $stmt->bindvalue(':last_update_date', current_time(), PDO::PARAM_INT);
     $stmt->bindvalue(':last_update_by', $_SESSION['username'], PDO::PARAM_INT);

     try {
      $stmt->execute();
      $$class->content_id = $dbc->connection->lastInsertId();
      echo "<br>Record is Successfully Saved in $class";
     } catch (PDOException $e) {
      $dbc->rollback = 1;
      echo "<br>Record couldnt be saved. Error @am_wo_work_bench_line @@ Line " . __LINE__ . $e->getMessage() . "<br/>";
     }
    }
   }
  }

  public function _xxafter_save() {
   global $db;
   global $dbc;
   if ((!empty($this->content_type_id)) && (!empty($this->content_id))) {
    $content_type = content_type::find_by_id($this->content_type_id);
    $class = content::$content_table_name = 'c_' . $content_type->content_type;

    $$class = new stdClass();
    $extra_fields = new_content_type::find_fields_by_id($content_type->content_type_id);

    $content_field_array = content::$content_field_array;
    foreach ($extra_fields as $records) {
     $content_field_array[] = $records->Field;
    }
    $$class->content_id = $this->content_id;
    $checkSql = " SELECT * FROM $class WHERE content_id = $this->content_id LIMIT 1";
    $checkResult = self::find_by_sql($checkSql);
    if (!empty($checkResult)) {
     $checkIfContentExists = true;
    } else {
     $checkIfContentExists = false;
    }
    if ($checkIfContentExists) {
     //Update the existing content
     $field_stmt = '';
     foreach ($content_field_array as $key => $value) {
      $field_stmt .= $value . ' = :' . $value . ', ';
     }
     $field_stmt = rtrim($field_stmt, ', ');
//write SQL
     $sql = " UPDATE ";
     $sql.= $class;
     $sql .= " SET ";
     $sql.= $field_stmt;
     $sql.= " WHERE content_id = :content_id ";
     $stmt = $dbc->connection->prepare(" $sql ");
     //Bind parameters
     foreach ($content_field_array as $key => $value) {
      if (!empty($_POST[$value][0])) {
       $stmt->bindParam(":$value", $_POST[$value][0]);
      } else {
       $stmt->bindvalue(":$value", null, PDO::PARAM_INT);
      }
     }
     try {
      $stmt->execute();
      $newMsg = "Record is Successfully Saved in $class";
     } catch (PDOException $e) {
      print "Record couldnt be saved!!: " . $e->getMessage() . "<br/>";
     }
    } else {
     $field_colums = '';
     foreach ($content_field_array as $key => $value) {
      $field_colums .= $value . ', ';
     }
     $field_colums = rtrim($field_colums, ', ');

     $field_values = '';
     foreach ($content_field_array as $key => $value) {
      $field_values .= ':' . $value . ', ';
     }
     $field_values = rtrim($field_values, ', ');
//write SQL
     $sql = " INSERT INTO ";
     $sql.= $class;
     $sql.= ' ( ';
     $sql.= $field_colums;
     $sql.= ' ) ';
     $sql.= 'VALUES';
     $sql.= ' ( ';
     $sql.= $field_values;
     $sql.= ' ) ';
//		print_r($_POST);
     $stmt = $dbc->connection->prepare(" $sql ");
     //Bind parameters
     foreach ($content_field_array as $key => $value) {
      if (!empty($_POST[$value][0])) {
       $stmt->bindParam(":$value", $_POST[$value][0]);
      } else {
       $stmt->bindvalue(":$value", null, PDO::PARAM_INT);
      }
     }
     $stmt->bindParam(':content_id', $$class->content_id);
     try {
      $stmt->execute();
      $$class->content_id = $dbc->connection->lastInsertId();
      $newMsg = "Record is Successfully Saved in $class";
     } catch (PDOException $e) {
      print "Record couldnt be saved!!: " . $e->getMessage() . "<br/>";
     }
    }
   }

   //url alias
   if ((!empty($content_type)) && (!empty($this->url_alias) || $content_type->auto_url_alias_cb == 1 )) {
    $url_alias = new ext_url_alias();
    $url_alias->alias = !empty($this->url_alias) ? $this->url_alias : $this->subject;
    $url_alias->content_id = $this->content_id;
    $url_alias->original_url = "content_id=$this->content_id&content_type=$content_type->content_type";
    $url_alias->ext_url_alias_id = !empty($this->ext_url_alias_id) ? $this->ext_url_alias_id : null;
    $url_alias->audit_trial();
    $url_alias->save();
   }
  }

 }

//end of am_wo_routing_line class
?>