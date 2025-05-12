//
// Copyright 2022 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// Modifications copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#ifndef PXR_IMAGING_HDUI_SCENE_INDEX_TREE_WIDGET_H
#define PXR_IMAGING_HDUI_SCENE_INDEX_TREE_WIDGET_H

#include "pxr/pxr.h"

#include "pxr/imaging/hd/sceneIndex.h"

#include <QTreeWidget>
#include <unordered_map>

PXR_NAMESPACE_OPEN_SCOPE

class HydraOpTreeItem;

//-----------------------------------------------------------------------------

class HydraOpTree
    : public QTreeWidget
    , public HdSceneIndexObserver
{
    Q_OBJECT;

public:
    HydraOpTree(QWidget *parent = Q_NULLPTR);

    void PrimsAdded(const HdSceneIndexBase &sender, const AddedPrimEntries &entries) override;
    void PrimsRemoved(const HdSceneIndexBase &sender, const RemovedPrimEntries &entries) override;
    void PrimsDirtied(const HdSceneIndexBase &sender, const DirtiedPrimEntries &entries) override;
#if PXR_VERSION >= 2408
    void PrimsRenamed(const HdSceneIndexBase &sender, const RenamedPrimEntries &entries) override;
#endif
    void SetSceneIndex(HdSceneIndexBaseRefPtr inputSceneIndex);
    void Requery(bool lazy = true);

Q_SIGNALS:
    void PrimSelected(const SdfPath &primPath, HdContainerDataSourceHandle dataSource);

    void PrimDirtied(const SdfPath &primPath, const HdDataSourceLocatorSet &locators);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    friend HydraOpTreeItem;

    void _RemoveSubtree(const SdfPath &primPath);

    void _AddPrimItem(const SdfPath &primPath, HydraOpTreeItem *item);

    HydraOpTreeItem *_GetPrimItem(const SdfPath &primPath, bool createIfNecessary = true);

    using _ItemMap = std::unordered_map<SdfPath, HydraOpTreeItem *, SdfPath::Hash>;

    _ItemMap _primItems;
    HdSceneIndexBaseRefPtr _inputSceneIndex;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
