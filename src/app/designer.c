/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/designer.c
 *
 * PURPOSE:
 *   Implement Studio authoring operations over the Framework designer engine while keeping semantic state outside GTK widgets.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This Studio adapter keeps product composition thin: it delegates reusable
 * behaviour to Framework services and adds only Studio-specific coordination.
 */

#include "umicom/studio/designer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct UmiStudioDesigner{UmiStudioDeclarative *declarative;UmiDesignerDocument *document;UmiDesignerHistory *history;UmiDesignerSelection selection;UmiDesignerLiveSourceWorkspace live_source;};

/* Recreate canonical source and preview text from the current semantic design. */
static UmiStatus synchronize_live_source(UmiStudioDesigner *designer)
{
    UmiDeclDiagnosticList diagnostics;
    char source[UMI_DESIGNER_LIVE_SOURCE_CAPACITY];
    char preview[UMI_DESIGNER_LIVE_PREVIEW_CAPACITY];
    size_t source_length = 0U;
    uint64_t revision;
    UmiStatus status;

    if (designer == NULL || designer->document == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    /* Source revisions describe editor updates, which may advance even when a
     * parsed document begins a fresh internal history. */
    revision = designer->live_source.source_revision + 1U;
    if (revision == 0U) return UMI_STATUS_CAPACITY_EXCEEDED;
    status = umi_decl_serialize(
        umi_designer_document_declarative(designer->document),
        source,
        sizeof(source),
        &source_length);
    /* The serializer length is checked even though its text is null-terminated,
     * making the capacity relationship explicit for future implementations. */
    if (status == UMI_STATUS_OK && source_length >= sizeof(source)) {
        status = UMI_STATUS_CAPACITY_EXCEEDED;
    }
    if (status == UMI_STATUS_OK) {
        status = umi_designer_live_source_workspace_update(
            &designer->live_source,
            source,
            revision,
            0U);
    }
    (void)memset(&diagnostics, 0, sizeof(diagnostics));
    if (status == UMI_STATUS_OK) {
        status = umi_studio_designer_preview(
            designer,
            preview,
            sizeof(preview),
            &diagnostics);
    }
    if (status == UMI_STATUS_OK) {
        return umi_designer_live_source_workspace_accept(
            &designer->live_source,
            revision,
            preview);
    }

    /* Keep a previous valid preview visible while exposing the first useful
     * diagnostic from the current document revision. */
    if (designer->live_source.pending) {
        const char *message = diagnostics.count > 0U
            ? diagnostics.items[0].message
            : umi_status_text(status);
        (void)umi_designer_live_source_workspace_reject(
            &designer->live_source,
            designer->live_source.source_revision,
            message);
    }
    return status;
}
/* Provide the release document operation used by this module and its client applications. */
static void release_document(UmiStudioDesigner *d){umi_designer_history_destroy(d->history);d->history=NULL;umi_designer_document_destroy(d->document);d->document=NULL;umi_designer_selection_clear(&d->selection);}
/* Provide the attach document operation used by this module and its client applications. */
static UmiStatus attach_document(UmiStudioDesigner *d,UmiDesignerDocument *doc){UmiStatus s;release_document(d);d->document=doc;s=umi_designer_history_create(doc,&d->history);/* Preserve the original failure result so the caller can respond to the correct cause. */ if(s!=UMI_STATUS_OK){umi_designer_document_destroy(doc);d->document=NULL;}/* Refresh every source-backed view only after the new document and history are both valid. */ if(s==UMI_STATUS_OK)s=synchronize_live_source(d);return s;}
/*
 * Initialise studio designer from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_designer_create(UmiStudioDeclarative *decl,UmiStudioDesigner **out){UmiStudioDesigner *d;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(decl==NULL||out==NULL)return UMI_STATUS_INVALID_ARGUMENT;*out=NULL;d=calloc(1U,sizeof(*d));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL)return UMI_STATUS_OUT_OF_MEMORY;d->declarative=decl;s=umi_designer_live_source_workspace_init(&d->live_source,UMI_DESIGNER_SOURCE_DECLARATIVE,UMI_DESIGNER_LIVE_DEFAULT_DEBOUNCE_MS);/* Create the initial semantic document only after its live-source state is valid. */ if(s==UMI_STATUS_OK)s=umi_studio_designer_new(d,"org.umicom.studio.designer");/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK){release_document(d);free(d);return s;}*out=d;return UMI_STATUS_OK;}
/* Release or reset state held by studio designer so the same storage can be reused safely. */
void umi_studio_designer_destroy(UmiStudioDesigner *d){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL)return;release_document(d);free(d);}
/*
 * Provide the studio designer bind commands operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_bind_commands(UmiStudioDesigner *d,UmiCommandRegistry *commands){return d!=NULL?umi_designer_register_commands(commands,d->history):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Provide the studio designer new operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_new(UmiStudioDesigner *d,const char *app){UmiDesignerDocument *doc=NULL;UmiDeclNode root;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_designer_document_create(app,&doc);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_decl_node_init(&root,"root","window","-");/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_decl_document_add_node(umi_designer_document_declarative(doc),&root);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK){umi_designer_document_destroy(doc);return s;}umi_designer_document_mark_saved(doc);return attach_document(d,doc);}
/*
 * Provide the studio designer load text operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_load_text(UmiStudioDesigner *d,const char *src,UmiDeclDiagnosticList *diags){UmiDeclDocument *decl=NULL;UmiDesignerDocument *doc=NULL;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_studio_declarative_parse(d->declarative,src,&decl,diags);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_designer_document_from_declarative(decl,1,&doc);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK){umi_decl_document_destroy(decl);return s;}umi_designer_document_mark_saved(doc);return attach_document(d,doc);}
/*
 * Provide the studio designer add component operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_add_component(UmiStudioDesigner *d,const char *id,const char *type,const char *parent){UmiDeclNode node;UmiDesignerOperation op;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_decl_node_init(&node,id,type,parent);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_designer_operation_add(&node,&op);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_designer_history_execute(d->history,&op);/* Keep Code, Mixed and Preview panels on the same successful revision. */ if(s==UMI_STATUS_OK)(void)synchronize_live_source(d);return s;}
/*
 * Provide the studio designer set property operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_set_property(
    UmiStudioDesigner *designer,
    const char *node_id,
    const char *property_name,
    const char *value_text)
{
    UmiDeclNode before;
    UmiDeclNode after;
    UmiDeclComponentDescriptor component;
    UmiDeclPropertyDescriptor property;
    UmiDesignerOperation operation;
    UmiDeclValueKind value_kind = UMI_DECL_VALUE_STRING;
    UmiStatus status;

    if (designer == NULL || node_id == NULL || property_name == NULL ||
        value_text == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_decl_document_find_node(
        umi_designer_document_declarative(designer->document),
        node_id,
        &before);

    /* Known properties retain their declared boolean, integer or real type.
     * Unknown forward-compatible properties remain strings as before. */
    if (status == UMI_STATUS_OK &&
        umi_decl_component_registry_find(
            umi_studio_declarative_components(designer->declarative),
            before.component_type,
            &component) == UMI_STATUS_OK &&
        umi_decl_component_find_property(
            &component,
            property_name,
            &property) == UMI_STATUS_OK) {
        status = umi_decl_property_validate_text(&property, value_text);
        value_kind = property.kind;
    }
    /* Copy a valid source node only after lookup and validation succeed. */
    if (status == UMI_STATUS_OK) {
        after = before;
        status = umi_decl_node_set_attribute(
            &after,
            property_name,
            value_kind,
            value_text);
    }
    if (status == UMI_STATUS_OK) {
        status = umi_designer_operation_set_property(
            &before,
            &after,
            property_name,
            &operation);
    }
    if (status == UMI_STATUS_OK) {
        status = umi_designer_history_execute(
            designer->history,
            &operation);
    }
    /* A successful property edit immediately publishes canonical source and preview. */
    if (status == UMI_STATUS_OK) (void)synchronize_live_source(designer);
    return status;
}
/*
 * Provide the studio designer select operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_select(UmiStudioDesigner *d,const char *id){return d!=NULL?umi_designer_selection_set_primary(&d->selection,id):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Provide the studio designer undo operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_undo(UmiStudioDesigner *d){UmiStatus s;if(d==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_designer_history_undo(d->history);/* Undo changes the shared semantic revision, so refresh every projection together. */ if(s==UMI_STATUS_OK)(void)synchronize_live_source(d);return s;}
/* Reapply the latest reverted designer operation so every frontend shares the same history semantics. */
UmiStatus umi_studio_designer_redo(UmiStudioDesigner *d){UmiStatus s;if(d==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_designer_history_redo(d->history);/* Redo changes the shared semantic revision, so refresh every projection together. */ if(s==UMI_STATUS_OK)(void)synchronize_live_source(d);return s;}
/*
 * Provide the studio designer preview operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_preview(UmiStudioDesigner *d,char *out,size_t cap,UmiDeclDiagnosticList *diags){return d!=NULL?umi_designer_preview(d->document,umi_studio_declarative_schema(d->declarative),out,cap,diags):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Provide the studio designer snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_snapshot(const UmiStudioDesigner *d,UmiStudioDesignerSnapshot *out){UmiDesignerPalette palette;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL||out==NULL)return UMI_STATUS_INVALID_ARGUMENT;(void)memset(out,0,sizeof(*out));s=umi_designer_document_snapshot(d->document,&out->document);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_designer_palette_build(umi_studio_declarative_components(d->declarative),NULL,&palette);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK){out->palette_items=palette.count;out->selected_items=d->selection.count;out->undo_count=umi_designer_history_undo_count(d->history);out->redo_count=umi_designer_history_redo_count(d->history);umi_designer_palette_dispose(&palette);}return s;}
/*
 * Provide the studio designer document operation used by this module and its client
 * applications.
 */
UmiDesignerDocument *umi_studio_designer_document(UmiStudioDesigner *d){return d!=NULL?d->document:NULL;}

/* Build one revision-consistent model for every visual-design tool panel. */
UmiStatus umi_studio_designer_workspace_model(
    const UmiStudioDesigner *designer,
    const char *palette_query,
    UmiDesignerWorkspaceModel *out_model)
{
    if (designer == NULL || designer->declarative == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_designer_workspace_model_project(
        designer->document,
        &designer->selection,
        umi_studio_declarative_components(designer->declarative),
        palette_query,
        out_model);
}

/* Choose a safe parent for a palette insertion. Container selections accept
 * children; other selections fall back to the document root. */
static UmiStatus palette_parent_id(
    UmiStudioDesigner *designer,
    char *out_parent_id,
    size_t parent_id_capacity)
{
    UmiDeclNode selected;
    UmiDeclComponentDescriptor component;
    UmiStatus status;

    if (designer == NULL || out_parent_id == NULL ||
        parent_id_capacity == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* A non-container selection cannot own a child, so root remains the
     * predictable insertion target used by a blank or ambiguous canvas. */
    status = umi_decl_copy_text(out_parent_id, parent_id_capacity, "root");
    if (status != UMI_STATUS_OK || designer->selection.primary[0] == '\0') {
        return status;
    }
    status = umi_decl_document_find_node(
        umi_designer_document_declarative(designer->document),
        designer->selection.primary,
        &selected);
    if (status != UMI_STATUS_OK) {
        return status;
    }
    status = umi_decl_component_registry_find(
        umi_studio_declarative_components(designer->declarative),
        selected.component_type,
        &component);
    if (status != UMI_STATUS_OK) {
        return status;
    }
    if (component.container) {
        status = umi_decl_copy_text(
            out_parent_id,
            parent_id_capacity,
            selected.node_id);
    }
    return status;
}

/* Generate an identifier without overwriting an existing component. The loop
 * is bounded by the document capacity, so malformed input cannot run forever. */
static UmiStatus palette_node_id(
    UmiStudioDesigner *designer,
    const char *component_type,
    char *out_node_id,
    size_t node_id_capacity)
{
    UmiDeclDocument *document;
    size_t suffix;

    if (designer == NULL || component_type == NULL || out_node_id == NULL ||
        node_id_capacity == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    document = umi_designer_document_declarative(designer->document);
    for (suffix = 1U; suffix <= UMI_DESIGNER_MAX_TREE_NODES; ++suffix) {
        UmiDeclNode existing;
        int written = snprintf(
            out_node_id,
            node_id_capacity,
            "%s-%zu",
            component_type,
            suffix);

        if (written < 0 || (size_t)written >= node_id_capacity) {
            return UMI_STATUS_CAPACITY_EXCEEDED;
        }
        if (umi_decl_document_find_node(
                document,
                out_node_id,
                &existing) == UMI_STATUS_NOT_FOUND) {
            return UMI_STATUS_OK;
        }
    }
    return UMI_STATUS_CAPACITY_EXCEEDED;
}

UmiStatus umi_studio_designer_add_palette_component(
    UmiStudioDesigner *designer,
    const char *component_type,
    char *out_node_id,
    size_t node_id_capacity)
{
    UmiDeclComponentDescriptor component;
    char parent_id[UMI_DECL_ID_CAPACITY];
    UmiStatus status;

    if (designer == NULL || component_type == NULL ||
        out_node_id == NULL || node_id_capacity == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    out_node_id[0] = '\0';
    status = umi_decl_component_registry_find(
        umi_studio_declarative_components(designer->declarative),
        component_type,
        &component);
    if (status == UMI_STATUS_OK) {
        status = palette_parent_id(
            designer,
            parent_id,
            sizeof(parent_id));
    }
    if (status == UMI_STATUS_OK) {
        status = palette_node_id(
            designer,
            component_type,
            out_node_id,
            node_id_capacity);
    }
    if (status == UMI_STATUS_OK) {
        status = umi_studio_designer_add_component(
            designer,
            out_node_id,
            component.component_type,
            parent_id);
    }
    if (status == UMI_STATUS_OK) {
        status = umi_studio_designer_select(designer, out_node_id);
    }
    if (status != UMI_STATUS_OK) {
        out_node_id[0] = '\0';
    }
    return status;
}

UmiStatus umi_studio_designer_set_selected_property(
    UmiStudioDesigner *designer,
    const char *property_name,
    const char *value_text)
{
    UmiDeclNode selected;
    UmiStatus status;

    if (designer == NULL || property_name == NULL || value_text == NULL ||
        designer->selection.primary[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Validate the text against the selected component's registered schema
     * before an undoable operation can change the document. */
    status = umi_decl_document_find_node(
        umi_designer_document_declarative(designer->document),
        designer->selection.primary,
        &selected);
    if (status == UMI_STATUS_OK) {
        status = umi_designer_inspector_validate_property(
            umi_studio_declarative_components(designer->declarative),
            selected.component_type,
            property_name,
            value_text);
    }
    if (status != UMI_STATUS_OK) return status;
    return umi_studio_designer_set_property(
        designer,
        designer->selection.primary,
        property_name,
        value_text);
}

/* Copy the live-source record so callers cannot mutate designer-owned state. */
UmiStatus umi_studio_designer_live_source_snapshot(
    const UmiStudioDesigner *designer,
    UmiDesignerLiveSourceWorkspace *out_workspace)
{
    if (designer == NULL || out_workspace == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_workspace = designer->live_source;
    return UMI_STATUS_OK;
}

/* Apply debounced source transactionally: invalid text never replaces the
 * semantic document or the last preview that was known to render correctly. */
UmiStatus umi_studio_designer_apply_live_source(
    UmiStudioDesigner *designer,
    const char *source,
    uint64_t now_millis,
    UmiDeclDiagnosticList *diagnostics)
{
    UmiDeclDocument *declarative = NULL;
    UmiDesignerDocument *document = NULL;
    uint64_t revision;
    UmiStatus status;

    if (designer == NULL || source == NULL || diagnostics == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    revision = designer->live_source.source_revision + 1U;
    if (revision == 0U) return UMI_STATUS_CAPACITY_EXCEEDED;
    status = umi_designer_live_source_workspace_update(
        &designer->live_source,
        source,
        revision,
        now_millis);
    if (status == UMI_STATUS_OK) {
        status = umi_studio_declarative_parse(
            designer->declarative,
            source,
            &declarative,
            diagnostics);
    }
    if (status == UMI_STATUS_OK) {
        status = umi_designer_document_from_declarative(
            declarative,
            1,
            &document);
    }
    if (status == UMI_STATUS_OK) {
        /* Ownership of the parsed declaration moves into the designer document. */
        declarative = NULL;
        /* Text entered in Code or Mixed mode is an unsaved design change. */
        umi_designer_document_mark_changed(document);
        status = attach_document(designer, document);
        document = NULL;
    }
    if (status != UMI_STATUS_OK) {
        const char *message = diagnostics->count > 0U
            ? diagnostics->items[0].message
            : umi_status_text(status);

        umi_decl_document_destroy(declarative);
        umi_designer_document_destroy(document);
        if (designer->live_source.pending) {
            (void)umi_designer_live_source_workspace_reject(
                &designer->live_source,
                revision,
                message);
        }
    }
    return status;
}
