<?php

namespace Drupal\farm_forest\Plugin\Asset\AssetType;

use Drupal\farm_entity\Plugin\Asset\AssetType\FarmAssetType;

/**
 * Provides the forest asset type.
 *
 * @AssetType(
 *   id = "forest",
 *   label = @Translation("Forest"),
 * )
 */
class Forest extends FarmAssetType {

  /**
   * {@inheritdoc}
   */
  public function buildFieldDefinitions() {
    $fields = [];

    // Forest type field.
    $options = [
      'type' => 'list_string',
      'label' => t('Forest type'),
      'allowed_values' => [
        'stand' => t('Natural stand'),
        'plantation' => t('Plantation'),
      ],
      'required' => TRUE,
      'weight' => [
        'form' => -50,
        'view' => -50,
      ],
    ];
    $fields['forest_type'] = $this->farmFieldFactory->bundleFieldDefinition($options);

    return $fields;
  }

}
