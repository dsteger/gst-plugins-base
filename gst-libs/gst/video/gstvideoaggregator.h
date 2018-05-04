/* Generic video aggregator plugin
 * Copyright (C) 2008 Wim Taymans <wim@fluendo.com>
 * Copyright (C) 2010 Sebastian Dröge <sebastian.droege@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_VIDEO_AGGREGATOR_H__
#define __GST_VIDEO_AGGREGATOR_H__

#ifndef GST_USE_UNSTABLE_API
#warning "The Video library from gst-plugins-bad is unstable API and may change in future."
#warning "You can define GST_USE_UNSTABLE_API to avoid this warning."
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/base/gstaggregator.h>
#include <gst/video/video-bad-prelude.h>

G_BEGIN_DECLS

typedef struct _GstVideoAggregator GstVideoAggregator;
typedef struct _GstVideoAggregatorClass GstVideoAggregatorClass;
typedef struct _GstVideoAggregatorPrivate GstVideoAggregatorPrivate;

#define GST_TYPE_VIDEO_AGGREGATOR_PAD (gst_video_aggregator_pad_get_type())
#define GST_VIDEO_AGGREGATOR_PAD(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VIDEO_AGGREGATOR_PAD, GstVideoAggregatorPad))
#define GST_VIDEO_AGGREGATOR_PAD_CAST(obj) ((GstVideoAggregatorPad *)(obj))
#define GST_VIDEO_AGGREGATOR_PAD_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_COMPOSITOR_PAD, GstVideoAggregatorPadClass))
#define GST_IS_VIDEO_AGGREGATOR_PAD(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_VIDEO_AGGREGATOR_PAD))
#define GST_IS_VIDEO_AGGREGATOR_PAD_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_VIDEO_AGGREGATOR_PAD))
#define GST_VIDEO_AGGREGATOR_PAD_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS((obj),GST_TYPE_VIDEO_AGGREGATOR_PAD,GstVideoAggregatorPadClass))

typedef struct _GstVideoAggregatorPad GstVideoAggregatorPad;
typedef struct _GstVideoAggregatorPadClass GstVideoAggregatorPadClass;
typedef struct _GstVideoAggregatorPadPrivate GstVideoAggregatorPadPrivate;

/**
 * GstVideoAggregatorPad:
 * @info: The #GstVideoInfo currently set on the pad
 * @buffer_vinfo: The #GstVideoInfo representing the type contained
 *                in @buffer
 * @aggregated_frame: The #GstVideoFrame ready to be used for aggregation
 *                    inside the aggregate_frames vmethod.
 * @zorder: The zorder of this pad
 */
struct _GstVideoAggregatorPad
{
  GstAggregatorPad parent;

  GstVideoInfo info;

  GstBuffer *buffer;

  GstVideoFrame *aggregated_frame;

  /* properties */
  guint zorder;
  gboolean ignore_eos;

  /* Subclasses can force an alpha channel in the (input thus output)
   * colorspace format */
  gboolean needs_alpha;

  /* < private > */
  GstVideoAggregatorPadPrivate *priv;

  gpointer          _gst_reserved[GST_PADDING];
};

/**
 * GstVideoAggregatorPadClass:
 *
 * @set_info: Lets subclass set a converter on the pad,
 *                 right after a new format has been negotiated.
 * @prepare_frame: Prepare the frame from the pad buffer (if any)
 *                 and sets it to @aggregated_frame
 * @clean_frame:   clean the frame previously prepared in prepare_frame
 */
struct _GstVideoAggregatorPadClass
{
  GstAggregatorPadClass parent_class;
  gboolean           (*set_info)              (GstVideoAggregatorPad * pad,
                                               GstVideoAggregator    * videoaggregator,
                                               GstVideoInfo          * current_info,
                                               GstVideoInfo          * wanted_info);

  gboolean           (*prepare_frame)         (GstVideoAggregatorPad * pad,
                                               GstVideoAggregator    * videoaggregator);

  void               (*clean_frame)           (GstVideoAggregatorPad * pad,
                                               GstVideoAggregator    * videoaggregator);

  gpointer          _gst_reserved[GST_PADDING_LARGE];
};

GST_VIDEO_BAD_API
GType gst_video_aggregator_pad_get_type   (void);

#define GST_TYPE_VIDEO_AGGREGATOR (gst_video_aggregator_get_type())
#define GST_VIDEO_AGGREGATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VIDEO_AGGREGATOR, GstVideoAggregator))
#define GST_VIDEO_AGGREGATOR_CAST(obj) ((GstVideoAggregator *)(obj))
#define GST_VIDEO_AGGREGATOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_VIDEO_AGGREGATOR, GstVideoAggregatorClass))
#define GST_IS_VIDEO_AGGREGATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_VIDEO_AGGREGATOR))
#define GST_IS_VIDEO_AGGREGATOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_VIDEO_AGGREGATOR))
#define GST_VIDEO_AGGREGATOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS((obj),GST_TYPE_VIDEO_AGGREGATOR,GstVideoAggregatorClass))

/**
 * GstVideoAggregator:
 * @info: The #GstVideoInfo representing the currently set
 * srcpad caps.
 */
struct _GstVideoAggregator
{
  GstAggregator aggregator;

  /*< public >*/
  /* Output caps */
  GstVideoInfo info;

  /* < private > */
  GstVideoAggregatorPrivate *priv;
  gpointer          _gst_reserved[GST_PADDING_LARGE];
};

/**
 * GstVideoAggregatorClass:
 * @update_caps:              Optional.
 *                            Lets subclasses update the #GstCaps representing
 *                            the src pad caps before usage.  Return %NULL to indicate failure.
 * @aggregate_frames:         Lets subclasses aggregate frames that are ready. Subclasses
 *                            should iterate the GstElement.sinkpads and use the already
 *                            mapped #GstVideoFrame from GstVideoAggregatorPad.aggregated_frame
 *                            or directly use the #GstBuffer from GstVideoAggregatorPad.buffer
 *                            if it needs to map the buffer in a special way. The result of the
 *                            aggregation should land in @outbuffer.
 * @get_output_buffer:        Optional.
 *                            Lets subclasses provide a #GstBuffer to be used as @outbuffer of
 *                            the #aggregate_frames vmethod.
 * @negotiated_caps:          Optional.
 *                            Notifies subclasses what caps format has been negotiated
 * @find_best_format:         Optional.
 *                            Lets subclasses decide of the best common format to use.
 **/
struct _GstVideoAggregatorClass
{
  /*< private >*/
  GstAggregatorClass parent_class;

  /*< public >*/
  GstCaps *          (*update_caps)               (GstVideoAggregator *  videoaggregator,
                                                   GstCaps            *  caps);
  GstFlowReturn      (*aggregate_frames)          (GstVideoAggregator *  videoaggregator,
                                                   GstBuffer          *  outbuffer);
  GstFlowReturn      (*get_output_buffer)         (GstVideoAggregator *  videoaggregator,
                                                   GstBuffer          ** outbuffer);
  void               (*find_best_format)          (GstVideoAggregator *  vagg,
                                                   GstCaps            *  downstream_caps,
                                                   GstVideoInfo       *  best_info,
                                                   gboolean           *  at_least_one_alpha);

  GstCaps           *sink_non_alpha_caps;

  /* < private > */
  gpointer            _gst_reserved[GST_PADDING_LARGE];
};

GST_VIDEO_BAD_API
GType gst_video_aggregator_get_type       (void);

G_END_DECLS
#endif /* __GST_VIDEO_AGGREGATOR_H__ */
