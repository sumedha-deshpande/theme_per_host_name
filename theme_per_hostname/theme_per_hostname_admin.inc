<?php

/**
 * @file
 * Contains logic to manage settings of theme_per_hostname Module.
 * It contains functions to Add, edit and delete the settings.
 * Also contains helper function to achieve the same.
 * 
 * @author iAssure International Technologies Pvt Ltd, Pune
 * 
 */
/*
 * Captures all enable themes used to populate the select list on config page.
 * 
 * @returns $themes - an array of enable themes
 */
function get_enablethemes() {
  $enable_themes = array();
  $all_themes = list_themes();

  foreach ($all_themes as $this_theme) {

    if ($this_theme->status == 1) {
      $enable_themes[$this_theme->name] = $this_theme->name;
    }
  }
  return $enable_themes;
}

/**
 * Implements hook_form()
 * 
 * @param type $form
 * @param type $form_state
 * @return string
 */
function theme_setup_form($form, &$form_state) {
  $host_name = "";
  $theme_name = "";
  $tid = "";
  $opration = "insert";

  try {
    if (arg(4) == "edit") {
      $record = db_select('hostname_theme_mapping', 'h')
          ->fields('h')
          ->condition('tid', arg(5), '=')
          ->execute()
          ->fetchAssoc();
      $opration = "edit";
      $tid = $record['tid'];
      $host_name = $record['hostname'];
      $theme_name = $record['themename'];
    }
  }
  catch (Exception $ex) {
    drupal_set_message(t('Error: %message', array('%message' => $ex->getMessage())), 'error');
  }
  $form['tid'] = array(
    '#type' => 'hidden',
    '#default_value' => $tid,
  );
  $form['operation'] = array(
    '#type' => 'hidden',
    '#default_value' => $opration,
  );
  $form['hostname'] = array(
    '#type' => 'textfield',
    '#title' => t('Host Name'),
    '#size' => 30,
    '#description' => t("Enter the Host-Name."),
    '#default_value' => $host_name,
    '#required' => TRUE,
  );

  $enabled_themes = get_enablethemes();

  $form['theme_options'] = array(
    '#type' => 'select',
    '#prefix' => 'Theme',
    '#options' => $enabled_themes,
    '#default_value' => $theme_name,
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
    '#submit' => array('theme_setup_form_submit'),
  );
  return $form;
}

/**
 * This will check duplicate hostname entry
 * Validates the form.
 */
function theme_setup_form_validate($form, &$form_state) {
    
    $result = db_select('hostname_theme_mapping', 'h')
      ->fields('h')
      ->condition('hostname', db_like($form_state['values']['hostname']) . '%', 'LIKE')
      ->execute()
      ->fetchAll();

    if ($result) {
      form_set_error('Duplicate Hostname' , 'Theme"' . $result[0]->themename . '" is already associated with Host-Name "' . $form_state['values']['hostname'] . '"');
    }
}


/**
 * This function takes care of dataentry form submission 
 * @param type $form
 * @param type $form_state
 */
function theme_setup_form_submit($form, &$form_state) {
  try {
    if ($form_state['values']['operation'] == "insert") {
      db_insert('hostname_theme_mapping')
          ->fields(array(
            'hostname' => $form_state['values']['hostname'],
            'themename' => $form_state['values']['theme_options'],
          ))->execute();
      drupal_set_message(t("Settings Saved successfully."));
    }
    elseif ($form_state['values']['operation'] == "edit") {
      $result = db_update('hostname_theme_mapping')
          ->fields(array(
            'hostname' => trim($form_state['values']['hostname']),
            'themename' => $form_state['values']['theme_options'],
          ))
          ->condition('tid', $form_state['values']['tid'], '=')
          ->execute();
      if ($result) {
        drupal_set_message(t("Settings updated successfully."));
      }
    }
    drupal_goto('admin/config/jolokia/theme_settings/listing');
  }
  catch (Exception $ex) {
    drupal_set_message(t('Error: %message', array('%message' => $ex->getMessage())), 'error');
  }
}

/**
 * This function is used to populate the listing page.
 * 
 * @param type $form
 * @param type $form_state
 * @return type
 */
function theme_setup_listing_form($form, &$form_state) {
  try {
    if (arg(5) == "delete") {
      $rec_deleted = db_delete('hostname_theme_mapping')
          ->condition('tid', arg(6))
          ->execute();
      if ($rec_deleted) {
        drupal_set_message(t("Record deleted successfully."));
      }
    }
  }
  catch (Exception $ex) {
    drupal_set_message(t('Error: %message', array('%message' => $ex->getMessage())), 'error');
  }

  $header = array(
    array('data' => t('Host-Name'), 'field' => 'hostname', 'sort' => 'ASC'),
    array('data' => t('Theme'), 'field' => 'themename', 'sort' => 'ASC'),
    array('data' => t('Edit'), 'field' => 'Edit', 'sort' => 'ASC'),
    array('data' => t('Delete'), 'field' => 'Delete', 'sort' => 'ASC'),
  );

  $records = db_select('hostname_theme_mapping', 'h')
      ->fields('h')
      ->extend('PagerDefault')
      ->limit(20)
      ->extend('TableSort')
      ->execute()
      ->fetchAll();
  $rec_count = count($records);
  if (!empty($records) && $rec_count != 0) {    
    foreach ($records as $row) {
      $edit_button = l(t('Edit'), "admin/config/jolokia/theme_settings/edit/" . $row->tid, $options = array());
      $delete_button = l(t('Delete'), "admin/config/jolokia/record/delete/" . $row->tid, $options = array());
      $data[] = array(
        $row->hostname,
        $row->themename,
        $edit_button,
        $delete_button,
      );
    }
  }
  if($rec_count == 0) {
    drupal_set_message(t("No mapping found. Associate theme with hostname."));
    return;
  }
  //Return result as "table format"
  return array(
    array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $data,
    ),
    array(
      '#theme' => 'pager',
      '#quantity' => 7,
    ),
  );
}
