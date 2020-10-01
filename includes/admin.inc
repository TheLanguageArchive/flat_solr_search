<?php

/**
 * Generating solr field added to dropdown id
 *
 * @param string $dropdown_id
 * @param string $field_id
 *
 * @return array
 */
function flat_solr_search_admin_generate_field($dropdown_id, $field_id) {

  return [

    '#prefix'      => '<div>',
    '#suffix'      => $field_id . '</div>',
    '#type'        => 'button',
    '#dropdown_id' => $dropdown_id,
    '#field_id'    => $field_id,
    '#name'        => 'delete_' . $dropdown_id . '_' . $field_id,
    '#value'       => t('Remove'),
    '#submit'      => ['flat_solr_search_admin_update_fields_submit'],
    '#ajax'        => [

        'callback'  => 'flat_solr_search_admin_update_fields',
        'wrapper'   => 'flat-solr-search-admin-wrapper',
        'event'     => 'click',
        'effect'    => 'none',
    ],
  ];
}

/**
 * Admin configuration form
 *
 * @param array $form
 * @param array $form_state
 *
 * @return array
 */
function flat_solr_search_admin_form($form, &$form_state) {

  $dropdowns = flat_solr_search_get_cmdi_dropdowns();

  $form['general'] = [

    '#type'   => 'fieldset',
    '#title'  => t('Solr Advanced Search Controlled Vocabularies'),
    '#tree'   => true,
    '#prefix' => '<div id="flat-solr-search-admin-wrapper">',
    '#suffix' => '</div>',
  ];

  if ($form_state['rebuild'] == false) {

    // building the initial form_state
    $fields = variable_get('flat_solr_search_fields');

    if (is_array($fields)) {

      foreach ($fields as $field => $dropdown) {
        $form_state['fields'][$dropdown['id']][$field] = $field;
      }
    }
  }

  if (isset($form_state['triggering_element'])) {

    if ($form_state['triggering_element']['#value'] == t('Remove')) {

      // removing field
      $dropdown_id = $form_state['triggering_element']['#dropdown_id'];
      $field_id    = $form_state['triggering_element']['#field_id'];

      if (isset($form_state['fields'][$dropdown_id]) && isset($form_state['fields'][$dropdown_id][$field_id])) {
        unset($form_state['fields'][$dropdown_id][$field_id]);
      }
    }

    if ($form_state['triggering_element']['#value'] == t('Add')) {

      // adding field
      $dropdown_id = $form_state['triggering_element']['#dropdown_id'];

      if (isset($form_state['values']['general'][$dropdown_id]) && isset($form_state['values']['general'][$dropdown_id]['solr_field']) && !empty($form_state['values']['general'][$dropdown_id]['solr_field'])) {

        $field_id = $form_state['values']['general'][$dropdown_id]['solr_field'];
        $form_state['fields'][$dropdown_id][$field_id] = $field_id;
      }
    }
  }

  foreach ($dropdowns as $id => $dropdown) {

      $form['general'][$id] = [

        'solr_field' => [

          '#type'              => 'textfield',
          '#title'             => $dropdown['title'],
          '#help_text'         => $dropdown['description'],
          '#size'              => 100,
          '#default_value'     => '',
          '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
        ],

        'button' => [

          '#type'        => 'button',
          '#dropdown_id' => $id,
          '#name'        => 'add_' . $id,
          '#value'       => t('Add'),
          '#submit'      => ['flat_solr_search_admin_update_fields_submit'],
          '#ajax'        => [

              'callback'  => 'flat_solr_search_admin_update_fields',
              'wrapper'   => 'flat-solr-search-admin-wrapper',
              'event'     => 'click',
              'effect'    => 'none',
          ],
        ],

        'fields' => [
        ],

        'separator' => [
          '#markup' => '<hr />',
        ],
      ];

      if (isset($form_state['fields'][$id])) {

        // adding fields to form (initial + changes through ajax)
        foreach ($form_state['fields'][$id] as $field) {
          $form['general'][$id]['fields'][$field] = flat_solr_search_admin_generate_field($id, $field);
        }
      }
  }

  $form['actions'] = [

    '#type'  => 'actions',
    'submit' => [

        '#type'   => 'submit',
        '#value'  => t('Save Configuration'),
        '#weight' => 0,
        '#submit' => ['flat_solr_search_submit'],
    ],
    'reset' => [

        '#type'   => 'submit',
        '#value'  => t('Reset to defaults'),
        '#weight' => 1,
        '#submit' => ['flat_solr_search_submit'],
    ],
];

  return $form;
}

/**
 * Which part of the form have to be returned
 * when ajax is completed
 *
 * @param array $form
 * @param array $form_state
 *
 * @return array
 */
function flat_solr_search_admin_update_fields($form, &$form_state) {
  return $form['general'];
}

/**
 * Rebuild form when ajax is done
 *
 * @param array $form
 * @param array $form_state
 */
function flat_solr_search_admin_update_fields_submit($form, &$form_state) {
  $form_state['rebuild'] = true;
}

/**
 * Saving solr data
 *
 * @param array $form
 * @param array $form_state
 */
function flat_solr_search_submit($form, &$form_state) {

  if ($form_state['triggering_element']['#value'] == t('Reset to defaults')) {

      $form_state['redirect'] = 'admin/config/flat_deposit/flat_solr_search';
      drupal_set_message(t('FLAT Solr Search config was successfully reset'));

  } else {

      flat_solr_search_admin_commit_fields($form_state['fields']);
      drupal_set_message(t('FLAT Solr Search config was successfully saved'));
  }
}

/**
 * Committing fields
 *
 * @param array $fields
 *
 * @return void
 */
function  flat_solr_search_admin_commit_fields($fields) {

  $dropdowns = flat_solr_search_get_cmdi_dropdowns();

  $commit = [];
  foreach ($fields as $dropdown_id => $fields) {

    foreach ($fields as $field) {

      if (isset($dropdowns[$dropdown_id])) {

        $commit[$field] = [

          'id'      => $dropdown_id,
          'options' => $dropdowns[$dropdown_id]['options'],
        ];
      }
    }
  }

  variable_set('flat_solr_search_fields', $commit);
}
